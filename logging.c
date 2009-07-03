#include <string.h>
#include <stdarg.h>

/* Need __USE_BSD for vsyslog on linux */
#ifndef __USE_BSD
# define __USE_BSD
#endif

#include <syslog.h>

#include "conflate.h"
#include "conflate_internal.h"
#include "conflate_convenience.h"

static char* lvl_name(enum conflate_log_level lvl)
{
    char *rv = NULL;

    switch(lvl) {
    case FATAL: rv = "FATAL"; break;
    case ERROR: rv = "ERROR"; break;
    case WARN: rv = "WARN"; break;
    case INFO: rv = "INFO"; break;
    case DEBUG: rv = "DEBUG"; break;
    }

    return rv;
}

static int prio_map(enum conflate_log_level lvl)
{
    int rv = LOG_EMERG;

    switch(lvl) {
    case FATAL: rv = LOG_CRIT; break;
    case ERROR: rv = LOG_ERR; break;
    case WARN: rv = LOG_WARNING; break;
    case INFO: rv = LOG_INFO; break;
    case DEBUG: rv = LOG_DEBUG; break;
    }

    return rv;
}

void conflate_syslog_logger(void *userdata, enum conflate_log_level lvl,
                            const char *msg, ...)
{
    char fmt[strlen(msg) + 16];
    snprintf(fmt, sizeof(fmt), "%s: %s\n", lvl_name(lvl), msg);

    va_list ap;
    va_start(ap, msg);
    vsyslog(prio_map(lvl), msg, ap);
    va_end(ap);
}

void conflate_stderr_logger(void *userdata, enum conflate_log_level lvl,
                            const char *msg, ...)
{
    char fmt[strlen(msg) + 16];
    snprintf(fmt, sizeof(fmt), "%s: %s\n", lvl_name(lvl), msg);

    va_list ap;
    va_start(ap, msg);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}
