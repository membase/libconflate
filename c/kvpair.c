#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "memagent.h"

kvpair_t* mk_kvpair(const char* k, char** v)
{
    kvpair_t* rv = calloc(1, sizeof(kvpair_t));
    assert(rv);

    rv->key = safe_strdup(k);
    if (v) {
        int i = 0;
        for (i = 0; v[i]; i++) {
            add_kvpair_value(rv, v[i]);
        }
    }

    return rv;
}

void add_kvpair_value(kvpair_t* pair, const char* value)
{
    assert(pair);
    assert(value);

    if (pair->allocated_values - pair->used_values >= 0) {
        pair->allocated_values = pair->allocated_values << 1;
        if (pair->allocated_values == 0) {
            pair->allocated_values = 4;
        }

        pair->values = realloc(pair->values,
                               sizeof(char*) * pair->allocated_values);
        assert(pair->values);
    }

    pair->values[pair->used_values++] = safe_strdup(value);
    pair->values[pair->used_values] = 0;
}

void free_kvpair(kvpair_t* pair)
{
    if (pair->next) {
        free_kvpair(pair->next);
    }

    free(pair->key);
    free_string_list(pair->values);
    free(pair);
}
