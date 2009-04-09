#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sqlite3.h>

#include "memagent.h"

#define HAS_LISTS   1
#define HAS_SERVERS 2

#define CREATE_LISTS "create table lists " \
    "(id integer primary key, name varchar(32), binding integer)"
#define CREATE_SERVERS "create table servers " \
    "(id integer primary key, list_id integer, host varchar(64), port integer)"

#define INS_LIST "insert into lists (name, binding) values (?, ?)"
#define INS_SERVER "insert into servers (list_id, host, port) values (?, ?, ?)"

/* safety-net */
#define MAX_STEPS 1024

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

static bool db_do(sqlite3 *db, const char* query)
{
    char* errmsg = NULL;
    bool rv = true;

    if (sqlite3_exec(db, query, NULL, NULL, &errmsg) != SQLITE_OK) {
        fprintf(stderr, "DB Error:  %s\n%s\n", errmsg, query);
        sqlite3_free(errmsg);
        rv = false;
    }

    return rv;
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
        if (db_do(db, query)) {
            fprintf(stderr, "Created table: %s\n", table_name);
        } else {
            fprintf(stderr, "DB error creating %s\n", table_name);
            rv = false;
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
    bool rv = false;
    int err = 0, i = 0, steps_run = 0;
    sqlite3 *db;
    sqlite3_stmt *ins_list = NULL, *ins_server = NULL;
    const char* unused;

    if ((err = sqlite3_open(filename, &db)) != SQLITE_OK) {
        goto finished;
    }

    if (!initialize_db(db)) {
        fprintf(stderr, "Error initializing tables.\n");
        goto finished;
    }

    if (!db_do(db, "begin transaction")) {
        goto finished;
    }

    /* Cleanup the existing stuff */

    if (!db_do(db, "delete from servers")) {
        goto finished;
    }

    if (!db_do(db, "delete from lists")) {
        goto finished;
    }

    /* Add new list */
    if (sqlite3_prepare_v2(db, INS_LIST, strlen(INS_LIST),
                           &ins_list, &unused) != SQLITE_OK) {
        goto finished;
    }
    /* Add new server */
    if (sqlite3_prepare_v2(db, INS_SERVER, strlen(INS_SERVER),
                           &ins_server, &unused) != SQLITE_OK) {
        goto finished;
    }

    /* OK, Add them all in */
    for (i = 0; lists[i]; i++) {
        memcached_server_list_t* list = lists[i];
        int j = 0, rc = 0;
        sqlite_int64 list_id;

        sqlite3_clear_bindings(ins_list);
        sqlite3_clear_bindings(ins_server);

        sqlite3_bind_text(ins_list, 1, list->name, strlen(list->name),
                          SQLITE_TRANSIENT);
        sqlite3_bind_int(ins_list, 2, list->binding);

        while ((rc = sqlite3_step(ins_list)) != SQLITE_DONE) {
            steps_run++;
            assert(steps_run < MAX_STEPS);
            fprintf(stderr, "list step result:  %d\n", rc);
        }

        list_id = sqlite3_last_insert_rowid(db);

        for (j = 0; list->servers[j]; j++) {
            memcached_server_t* server = list->servers[j];

            sqlite3_bind_int64(ins_server, 1, list_id);
            sqlite3_bind_text(ins_server, 2, server->host, strlen(server->host),
                              SQLITE_TRANSIENT);
            sqlite3_bind_int(ins_server, 3, server->port);

            while ((rc = sqlite3_step(ins_server)) != SQLITE_DONE) {
                steps_run++;
                assert(steps_run < MAX_STEPS);
                fprintf(stderr, "server step result:  %d\n", rc);
            }

            sqlite3_reset(ins_server);
        }

        sqlite3_reset(ins_list);
    }

    if (!db_do(db, "commit")) {
        goto finished;
    }

    rv = true;

 finished:
    if (sqlite3_errcode(db) != SQLITE_OK) {
        fprintf(stderr, "DB error %d:  %s\n", sqlite3_errcode(db), sqlite3_errmsg(db));
    }

    if (ins_list) {
        sqlite3_finalize(ins_list);
    }
    if (ins_server) {
        sqlite3_finalize(ins_server);
    }
    sqlite3_close(db);

    return rv;
}
