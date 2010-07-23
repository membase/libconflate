#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <curl/curl.h>

#include "conflate.h"
#include "rest.h"
#include "conflate_internal.h"

long curl_init_flags = CURL_GLOBAL_ALL;
#ifdef __WIN32__
curl_init_flags = curl_init_flags & CURL_GLOBAL_WIN32;
#endif

struct response_buffer {
    char *data;
    size_t bytes_used;
    size_t buffer_size;
    struct response_buffer *next;
};

struct response_buffer *response_buffer_head = NULL;
struct response_buffer *cur_response_buffer = NULL;

static struct response_buffer *mk_response_buffer(size_t size) {
    struct response_buffer *r = (struct response_buffer *)malloc(sizeof(struct response_buffer));
    assert(r);
    memset(r,0,sizeof(struct response_buffer));
    r->data = malloc(size);
    assert(r->data);
    r->bytes_used = 0;
    r->buffer_size = size;
    r->next = NULL;
    return r;
}

static void free_response(struct response_buffer *response) {
    if (!response) {
        return;
    }
    if (!response->next) {
        free_response(response->next);
    }
    free(response);
}

static struct response_buffer *write_data_to_buffer(struct response_buffer *buffer, const char *data, size_t len) {
    size_t bytes_written = 0;
    while (bytes_written < len) {
        size_t bytes_to_write = (len - bytes_written);
        size_t space = buffer->buffer_size - buffer->bytes_used;
        if (space == 0) {
            struct response_buffer *new_buffer = mk_response_buffer(buffer->buffer_size);
            buffer->next = new_buffer;
            buffer = new_buffer;
        }
        if (bytes_to_write > space) {
            bytes_to_write = space;
        }
        char *d = buffer->data;
        d = &d[buffer->bytes_used];
        memcpy(d,&data[bytes_written],bytes_to_write);
        bytes_written += bytes_to_write;
        buffer->bytes_used += bytes_to_write;
    }
    return buffer;
}

static char *assemble_complete_response(struct response_buffer *response_head) {
    if (response_head == NULL) {
        return NULL;
    }

    //figure out how big the message is
    struct response_buffer *cur_buffer = response_head;
    size_t response_size = 0;
    char *response = NULL;
    while (cur_buffer) {
        response_size += cur_buffer->bytes_used;
        cur_buffer = cur_buffer->next;
    }

    //create buffer
    response = malloc(response_size + 1);
    assert(response);

    //populate buffer
    cur_buffer = response_head;
    char *ptr = response;
    while (cur_buffer) {
        memcpy(ptr, cur_buffer->data, cur_buffer->bytes_used);
        ptr += cur_buffer->bytes_used;
        cur_buffer = cur_buffer->next;
    }

    response[response_size] = '\0';

    return response;
}

static bool pattern_ends_with(const char *pattern, const char * target, size_t target_size) {
	assert(target);
	assert(pattern);

	size_t pattern_size = strlen(pattern);
	if (target_size < pattern_size) {
		return false;
	}
	return memcmp(&target[target_size - pattern_size], pattern, pattern_size) == 0;
}

static void process_new_config(conflate_handle_t *conf_handle) {
    //construct the new config from its components
    char *values[2];
    values[0] = assemble_complete_response(response_buffer_head);
    values[1] = NULL;

    free_response(response_buffer_head);
    response_buffer_head = NULL;
    cur_response_buffer = NULL;

    if (values[0] == NULL) {
        return;
    }

    kvpair_t *kv = mk_kvpair(CONFIG_KEY, values);

    //execute the provided call back
    void (*call_back)(void*, kvpair_t*) = conf_handle->conf->new_config;
    call_back(conf_handle->conf->userdata, kv);

    //clean up
    free_kvpair(kv);
    free(values[0]);

    response_buffer_head = mk_response_buffer(RESPONSE_BUFFER_SIZE);
    cur_response_buffer = response_buffer_head;
}

static size_t handle_response(void *data, size_t s, size_t num, void *stream) {
    conflate_handle_t *c_handle = (conflate_handle_t *)stream;
    size_t size = s * num;
    bool end_of_message = pattern_ends_with(END_OF_CONFIG,data,size);
    cur_response_buffer = write_data_to_buffer(cur_response_buffer, data, size);
    if (end_of_message) {
        process_new_config(c_handle);
    }
    return size;
}

static void setup_handle(CURL *handle, char *user, char *pass, char *uri,
                         char *uri_suffix, conflate_handle_t *chandle,
                         size_t (handle_response)(void *, size_t, size_t, void *)) {
    size_t buff_size = strlen(uri) + strlen (uri_suffix) + 1;
    char *url = (char *) malloc(buff_size);
    if (url != NULL) {
        snprintf(url,buff_size,"%s%s",uri,uri_suffix);

        assert(curl_easy_setopt(handle,CURLOPT_WRITEDATA,chandle) == CURLE_OK);
        assert(curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,handle_response) == CURLE_OK);
        assert(curl_easy_setopt(handle,CURLOPT_URL,url) == CURLE_OK);

        if (user) {
            buff_size = strlen(user) + strlen(pass) + 2;
            char *userpasswd = (char *) malloc(buff_size);
            assert(userpasswd);
            snprintf(userpasswd,buff_size,"%s:%s",user,pass);
            assert(curl_easy_setopt(handle,CURLOPT_HTTPAUTH,CURLAUTH_BASIC) == CURLE_OK);
            assert(curl_easy_setopt(handle,CURLOPT_USERPWD,userpasswd) == CURLE_OK);
            free (userpasswd);
        }
        assert(curl_easy_setopt(handle,CURLOPT_HTTPGET,1) == CURLE_OK);

        free(url);
    }
}

void* run_rest_conflate(void *arg) {
    conflate_handle_t* handle = (conflate_handle_t*)arg;

    /* prep the buffers used to hold the config */
    response_buffer_head = mk_response_buffer(RESPONSE_BUFFER_SIZE);
    cur_response_buffer = response_buffer_head;

    /* Before connecting and all that, load the stored config */
    kvpair_t* conf = load_kvpairs(handle, handle->conf->save_path);
    if (conf) {
        handle->conf->new_config(handle->conf->userdata, conf);
        free_kvpair(conf);
    }

    /* init curl */
    assert(curl_global_init(curl_init_flags) == CURLE_OK);
    CURL *curl_handle = curl_easy_init();
    assert(curl_handle);

    bool succeeding = true;

    while (succeeding) {
        succeeding = false;

        char *urls = strdup(handle->conf->host); // Might be a '|' delimited list of url's.
        char *next = urls;

        while (next != NULL) {
            char *url = strsep(&next, "|");

            setup_handle(curl_handle,
                         handle->conf->jid, // The auth user.
                         handle->conf->pass, // The auth password.
                         url, "", handle, handle_response);

            if (curl_easy_perform(curl_handle) == 0) {
                /* We reach here if the REST server didn't provide a
                   streaming JSON response and so we need to process
                   the just-one-JSON response */
                process_new_config(handle);
                succeeding = true;
                next = NULL; // Restart at the beginning of the urls list.
            }
        }

        sleep(1); // Don't overload the REST servers with tons of retries.

        free(urls);
    }

    free_response(response_buffer_head);
    response_buffer_head = NULL;
    cur_response_buffer = NULL;

    curl_easy_cleanup(curl_handle);

    printf("ERROR: could not contact REST server(s): %s\n", handle->conf->host);
    exit(1);

    return NULL;
}
