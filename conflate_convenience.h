/* Convenience stuff that shouldn't be part of the API */

#define CONFLATE_LOG(handle, level, msg, args...)                   \
    handle->conf->log(handle->conf->userdata, level, msg, ##args)
