#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sqlite3.h>

#include "memagent.h"

memcached_server_list_t** load_server_lists(const char *filename)
{
    return NULL;
}

bool save_server_lists(memcached_server_list_t** lists, const char *filename)
{
    int err = 0;
    sqlite3 *db;

    if ((err = sqlite3_open(filename, &db)) != SQLITE_OK) {
        goto broken;
    }

    return true;

 broken:
    sqlite3_close(db);
    fprintf(stderr, "DB error:  %s\n", sqlite3_errmsg(db));
    return false;
}
