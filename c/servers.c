#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "memagent.h"

memcached_server_list_t* create_server_list(const char *name, int port)
{
    memcached_server_list_t* rv = calloc(sizeof(memcached_server_list_t), 1);
    assert(rv);
    rv->name = safe_strdup(name);
    rv->binding = port;
    rv->server_allocation = 8;
    rv->servers = calloc(sizeof(memcached_server_t*), rv->server_allocation);
    return rv;
}

static memcached_server_t* create_server_from_url(const char *url)
{
    char *u = safe_strdup(url);
    char *pos = NULL, *cur = NULL;
    memcached_server_t* rv = calloc(sizeof(memcached_server_t), 1);

    assert(rv);

    cur = strtok_r(u, ":", &pos);
    rv->host = safe_strdup(cur);

    cur = strtok_r(NULL, ":", &pos);
    if (cur && strlen(cur) > 0) {
        rv->port = strtol(cur, NULL, 10);
        /* Default to the memcached port */
        if (rv->port < 1 || rv->port > 65535) {
            rv->port = 11211;
        }
    }

    free(u);
    return rv;
}

/* Returns the newly created server object */
memcached_server_t* append_server(memcached_server_list_t *in, char* url)
{
    assert(in);
    assert(url);

    memcached_server_t* rv = create_server_from_url(url);

    if (in->server_next >= in->server_allocation) {
        in->server_allocation <<= 1;
        in->servers = realloc(in->servers,
                              in->server_allocation * sizeof(memcached_server_t*));
        assert(in->servers);
    }

    in->servers[in->server_next++] = rv;
    in->servers[in->server_next] = NULL;

    return rv;
}

static void free_server(memcached_server_t* server)
{
    assert(server);
    assert(server->host);

    free(server->host);
    free(server);
}

void free_server_list(memcached_server_list_t* server_list)
{
    int i = 0;

    assert(server_list);
    assert(server_list->name);

    free(server_list->name);
    for (i = 0; server_list->servers[i]; i++) {
        free_server(server_list->servers[i]);
    }
    free(server_list->servers);
    free(server_list);
}
