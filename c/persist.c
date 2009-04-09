#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sqlite3.h>

#include "memagent.h"

#define HAS_LISTS   1
#define HAS_SERVERS 2

#define CREATE_LISTS "create table lists " \
    "(id integer, name varchar(32), binding integer)"
#define CREATE_SERVERS "create table servers " \
    "(id integer, list_id integer, host varchar(64), port integer)"

memcached_server_list_t** load_server_lists(const char *filename)
{
    return NULL;
}

static int set_table_mask(void* arg, int n, char **vals, char **cols)
{
    int* out = (int*)arg;
    assert(n == 1);

    if (strcmp(vals[0], "lists") == 0) {
        *out |= HAS_LISTS;
    } else if(strcmp(vals[0], "servers") == 0) {
        *out |= HAS_SERVERS;
    } else {
        fprintf(stderr, "Warning:  Unknown table:  %s\n", vals[0]);
    }

    return SQLITE_OK;
}

static char* get_table_name(int flag) {
    switch (flag) {
    case HAS_LISTS:
        return "lists";
    case HAS_SERVERS:
        return "servers";
    default:
        assert(false);
    }
}

static bool maybe_create_table(sqlite3 *db, int flag, int flags, const char* query)
{
    assert(db);
    assert(query);

    bool rv = true;
    char* table_name = get_table_name(flag);

    if (flag & flags) {
        fprintf(stderr, "Table %s already exists\n", table_name);
    } else {
        char* errmsg = NULL;
        if (sqlite3_exec(db, query, NULL, NULL, &errmsg) != SQLITE_OK) {
            fprintf(stderr, "DB error creating %s:  %s\n", table_name, errmsg);
            sqlite3_free(errmsg);
            rv = false;
        } else {
            fprintf(stderr, "Created table: %s\n", table_name);
        }
    }

    return rv;
}

static bool initialize_db(sqlite3 *db)
{
    int found_tables = 0;
    bool rv = true;
    char *errmsg = NULL;
    char* query = "SELECT name FROM sqlite_master "
        "WHERE type='table' "
        "ORDER BY name;";

    assert(db);

    if (sqlite3_exec(db, query, set_table_mask, &found_tables, &errmsg) != SQLITE_OK) {
        fprintf(stderr, "DB error:  %s\n", errmsg);
        sqlite3_free(errmsg);
    } else {
        rv &= maybe_create_table(db, HAS_LISTS, found_tables, CREATE_LISTS);
        rv &= maybe_create_table(db, HAS_SERVERS, found_tables, CREATE_SERVERS);
    }
}

bool save_server_lists(memcached_server_list_t** lists, const char *filename)
{
    int err = 0;
    sqlite3 *db;

    if ((err = sqlite3_open(filename, &db)) != SQLITE_OK) {
        goto broken;
    }

    if (!initialize_db(db)) {
        fprintf(stderr, "Error initializing tables.\n");
        goto broken;
    }

    return true;

 broken:
    sqlite3_close(db);
    fprintf(stderr, "DB error:  %s\n", sqlite3_errmsg(db));
    return false;
}
