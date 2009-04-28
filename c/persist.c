#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sqlite3.h>

#include "conflate.h"

#define HAS_KEYS   1
#define HAS_VALUES 2

#define CREATE_KEYS "create table keys " \
    "(id integer primary key, name varchar(32))"
#define CREATE_VALUES "create table vals " \
    "(key_id integer, value varchar(64))"

#define INS_KEYS "insert into keys (name) values (?)"
#define INS_VALS "insert into vals (key_id, value) values (?, ?)"

#define LOAD_KVPAIRS "select k.name, v.value " \
    "from keys k join vals v on (k.id = v.key_id)"

/* safety-net */
#define MAX_STEPS 1024

static int set_table_mask(void* arg, int n, char **vals, char **cols)
{
    int* out = (int*)arg;
    assert(n == 1);

    if (strcmp(vals[0], "keys") == 0) {
        *out |= HAS_KEYS;
    } else if(strcmp(vals[0], "vals") == 0) {
        *out |= HAS_VALUES;
    } else {
        fprintf(stderr, "Warning:  Unknown table:  %s\n", vals[0]);
    }

    return SQLITE_OK;
}

static char* get_table_name(int flag) {
    switch (flag) {
    case HAS_KEYS:
        return "keys";
    case HAS_VALUES:
        return "vals";
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
        rv &= maybe_create_table(db, HAS_KEYS, found_tables, CREATE_KEYS);
        rv &= maybe_create_table(db, HAS_VALUES, found_tables, CREATE_VALUES);
    }

    return rv;
}

bool save_kvpairs(kvpair_t* kvpair, const char *filename)
{
    bool rv = false;
    int err = 0, steps_run = 0;
    sqlite3 *db;
    sqlite3_stmt *ins_keys = NULL, *ins_vals = NULL;
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

    if (!db_do(db, "delete from keys")) {
        goto finished;
    }

    if (!db_do(db, "delete from vals")) {
        goto finished;
    }

    /* Add the keys */
    if (sqlite3_prepare_v2(db, INS_KEYS, strlen(INS_KEYS),
                           &ins_keys, &unused) != SQLITE_OK) {
        goto finished;
    }
    /* Add the values */
    if (sqlite3_prepare_v2(db, INS_VALS, strlen(INS_VALS),
                           &ins_vals, &unused) != SQLITE_OK) {
        goto finished;
    }

    /* OK, Add them all in */
    while (kvpair) {
        int j = 0, rc = 0;
        sqlite_int64 key_id;

        sqlite3_clear_bindings(ins_keys);
        sqlite3_clear_bindings(ins_vals);

        sqlite3_bind_text(ins_keys, 1, kvpair->key, strlen(kvpair->key),
                          SQLITE_TRANSIENT);

        while ((rc = sqlite3_step(ins_keys)) != SQLITE_DONE) {
            steps_run++;
            assert(steps_run < MAX_STEPS);
            fprintf(stderr, "keys step result:  %d\n", rc);
        }

        key_id = sqlite3_last_insert_rowid(db);

        for (j = 0; kvpair->values[j]; j++) {
            char* val = kvpair->values[j];

            sqlite3_bind_int64(ins_vals, 1, key_id);
            sqlite3_bind_text(ins_vals, 2, val, strlen(val), SQLITE_TRANSIENT);

            while ((rc = sqlite3_step(ins_vals)) != SQLITE_DONE) {
                steps_run++;
                assert(steps_run < MAX_STEPS);
                fprintf(stderr, "vals step result:  %d\n", rc);
            }

            sqlite3_reset(ins_vals);
        }

        sqlite3_reset(ins_keys);

        kvpair = kvpair -> next;
    }

    if (!db_do(db, "commit")) {
        goto finished;
    }

    rv = true;

 finished:
    if (sqlite3_errcode(db) != SQLITE_OK) {
        fprintf(stderr, "DB error %d:  %s\n", sqlite3_errcode(db), sqlite3_errmsg(db));
    }

    if (ins_keys) {
        sqlite3_finalize(ins_keys);
    }
    if (ins_vals) {
        sqlite3_finalize(ins_vals);
    }
    sqlite3_close(db);

    return rv;
}

static int append_kvpair_from_db(void* arg, int n, char **vals, char **cols)
{
    kvpair_t** pairs = (kvpair_t**)arg;
    kvpair_t* pair = *pairs;

    /* Search for the pair with the name we're looking for */
    while (pair && strcmp(pair->key, vals[0]) != 0) {
        pair = pair->next;
    }

    /* If such a list doesn't already exist, one will be assigned to you */
    if (!pair) {
        pair = mk_kvpair(vals[0], NULL);
        pair->next = *pairs;
        *pairs = pair;
    }

    add_kvpair_value(pair, vals[1]);

    return SQLITE_OK;
}

kvpair_t* load_kvpairs(const char *filename)
{
    char* errmsg = NULL;
    sqlite3 *db = NULL;
    kvpair_t* pairs = NULL;

    if (sqlite3_open(filename, &db) != SQLITE_OK) {
        goto finished;
    }

    if (sqlite3_exec(db, LOAD_KVPAIRS, &append_kvpair_from_db, &pairs, &errmsg) != SQLITE_OK) {
        goto finished;
    }

 finished:
    if (sqlite3_errcode(db) != SQLITE_OK) {
        fprintf(stderr, "DB error %d:  %s\n", sqlite3_errcode(db), sqlite3_errmsg(db));
        if (errmsg) {
            fprintf(stderr, "  %s\n", errmsg);
            sqlite3_free(errmsg);
        }
    }

    sqlite3_close(db);

    return pairs;
}
