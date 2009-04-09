#include <assert.h>
#include <string.h>

char* safe_strdup(const char* in) {
    char *rv = NULL;
    assert(in);
    rv = strdup(in);
    assert(rv);
    return rv;
}
