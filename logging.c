#include "config.h"
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_SYSLOG_H

/* Need __USE_BSD for vsyslog on linux */
#ifndef __USE_BSD
# define __USE_BSD
#endif

#include <syslog.h>
#endif

#include "conflate.h"
#include "conflate_internal.h"
#include "conflate_convenience.h"

static char* lvl_name(enum conflate_log_level lvl)
{
    char *rv = NULL;

    switch(lvl) {
    case LOG_LVL_FATAL: rv = "FATAL"; break;
    case LOG_LVL_ERROR: rv = "ERROR"; break;
    case LOG_LVL_WARN: rv = "WARN"; break;
    case LOG_LVL_INFO: rv = "INFO"; break;
    case LOG_LVL_DEBUG: rv = "DEBUG"; break;
    }

    return rv;
}

#ifdef HAVE_SYSLOG_H
static int prio_map(enum conflate_log_level lvl)
{
    int rv = LOG_EMERG;

    switch(lvl) {
    case LOG_LVL_FATAL: rv = LOG_CRIT; break;
    case LOG_LVL_ERROR: rv = LOG_ERR; break;
    case LOG_LVL_WARN: rv = LOG_WARNING; break;
    case LOG_LVL_INFO: rv = LOG_INFO; break;
    case LOG_LVL_DEBUG: rv = LOG_DEBUG; break;
    }

    return rv;
}

void conflate_syslog_logger(void *userdata, enum conflate_log_level lvl,
                            const char *msg, ...)
{
    (void)userdata;
    char fmt[strlen(msg) + 16];
    snprintf(fmt, sizeof(fmt), "%s: %s\n", lvl_name(lvl), msg);

    va_list ap;
    va_start(ap, msg);
    vsyslog(prio_map(lvl), msg, ap);
    va_end(ap);
}
#endif

void conflate_stderr_logger(void *userdata, enum conflate_log_level lvl,
                            const char *msg, ...)
{
    (void)userdata;
    char fmt[strlen(msg) + 16];
    snprintf(fmt, sizeof(fmt), "%s: %s\n", lvl_name(lvl), msg);

    va_list ap;
    va_start(ap, msg);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}
