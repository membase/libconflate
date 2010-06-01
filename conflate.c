#include <alarm.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "conflate.h"
#include "conflate_internal.h"
#include "rest.h"

/* Randomly generated by a fair dice roll */
#define INITIALIZATION_MAGIC 142285011
#define HTTP_PREFIX "HTTP:"

void *run_conflate(void *);


conflate_config_t* dup_conf(conflate_config_t c) {
    conflate_config_t *rv = calloc(sizeof(conflate_config_t), 1);
    assert(rv);

    rv->jid = safe_strdup(c.jid);
    rv->pass = safe_strdup(c.pass);
    if (c.host) {
        rv->host = safe_strdup(c.host);
    }
    rv->software = safe_strdup(c.software);
    rv->version = safe_strdup(c.version);
    rv->save_path = safe_strdup(c.save_path);
    rv->userdata = c.userdata;
    rv->log = c.log;
    rv->new_config = c.new_config;

    rv->initialization_marker = (void*)INITIALIZATION_MAGIC;

    return rv;
}

void init_conflate(conflate_config_t *conf)
{
    assert(conf);
    memset(conf, 0x00, sizeof(conflate_config_t));
    conf->log = conflate_syslog_logger;
    conf->initialization_marker = (void*)INITIALIZATION_MAGIC;
}

bool start_conflate(conflate_config_t conf) {

    void *(*run_func)(void*) = NULL;

    /* Don't start if we don't believe initialization has occurred. */
    if (conf.initialization_marker != (void*)INITIALIZATION_MAGIC) {
        assert(conf.initialization_marker == (void*)INITIALIZATION_MAGIC);
        return false;
    }

    conflate_handle_t *handle = calloc(1, sizeof(conflate_handle_t));
    assert(handle);

    if (strncmp(HTTP_PREFIX, conf.host, strlen(HTTP_PREFIX))) {
        run_func = &run_rest_conflate;
    } else {
        run_func = &run_conflate;
        conflate_init_commands();

#ifdef CONFLATE_USE_XMPP
        xmpp_initialize();
#endif
    }

    handle->alarms = init_alarmqueue();

    handle->conf = dup_conf(conf);

    if (pthread_create(&handle->thread, NULL, run_func, handle) == 0) {
        return true;
    } else {
        perror("pthread_create");
    }

    return false;
}
