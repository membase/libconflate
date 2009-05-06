#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "conflate.h"

char* safe_strdup(const char* in) {
    char *rv = NULL;
    assert(in);
    rv = strdup(in);
    assert(rv);
    return rv;
}

void free_string_list(char **vals)
{
    int i = 0;
    for (i = 0; vals[i]; i++) {
        free(vals[i]);
    }
    free(vals);
}
