/* Convenience stuff that shouldn't be part of the API */
#ifndef CONFLATE_CONVENIENCE_H
#define CONFLATE_CONVENIENCE_H 1

#define CONFLATE_LOG(handle, level, ...) \
   handle->conf->log(handle->conf->userdata, level, __VA_ARGS__)

#endif
