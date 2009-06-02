#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sqlite3.h>

#include "conflate.h"
#include "conflate_convenience.h"

#define HAS_KEYS    1
#define HAS_VALUES  2
#define HAS_PRIVATE 4

#define CREATE_KEYS "create table keys "        \
    "(id integer primary key, name varchar(32))"
#define CREATE_VALUES "create table vals "      \
    "(key_id integer, value varchar(64))"
#define CREATE_PRIVATE "create table private "            \
    "(key varchar(256) primary key on conflict replace, " \
    "value text not null)"

#define INS_KEYS "insert into keys (name) values (?)"
#define INS_VALS "insert into vals (key_id, value) values (?, ?)"
#define INS_PRIV "insert into private (key, value) values (?, ?)"

#define LOAD_KVPAIRS "select k.name, v.value " \
    "from keys k join vals v on (k.id = v.key_id)"

/* safety-net */
#define MAX_STEPS 1024

static int run_mod_steps(conflate_handle_t *handle, sqlite3 *db,
                         sqlite3_stmt *statement)
    __attribute__ ((warn_unused_result, nonnull(1, 2, 3)));

struct table_mask_userdata {
    int found;
    conflate_handle_t *handle;
};

static int set_table_mask(void* arg, int n, char **vals, char **cols)
{
    struct table_mask_userdata *udata = (struct table_mask_userdata*)arg;
    assert(n == 1);

    if (strcmp(vals[0], "keys") == 0) {
        udata->found |= HAS_KEYS;
    } else if(strcmp(vals[0], "vals") == 0) {
        udata->found |= HAS_VALUES;
    } else if(strcmp(vals[0], "private") == 0) {
        udata->found |= HAS_PRIVATE;
    } else {
        CONFLATE_LOG(udata->handle, WARN, "Unknown table:  %s", vals[0]);
    }

    return SQLITE_OK;
}

static char* get_table_name(int flag) {
    switch (flag) {
    case HAS_KEYS:
        return "keys";
    case HAS_VALUES:
        return "vals";
    case HAS_PRIVATE:
        return "private";
    default:
        assert(false);
    }
}

static bool db_do(conflate_handle_t *handle, sqlite3 *db, const char* query)
{
    char* errmsg = NULL;
    bool rv = true;

    if (sqlite3_exec(db, query, NULL, NULL, &errmsg) != SQLITE_OK) {
        CONFLATE_LOG(handle, ERROR, "DB Error:  %s\n%s", errmsg, query);
        sqlite3_free(errmsg);
        rv = false;
    }

    return rv;
}

static bool maybe_create_table(conflate_handle_t *handle,
                               sqlite3 *db, int flag, int flags,
                               const char* query)
{
    assert(db);
    assert(query);

    bool rv = true;
    char* table_name = get_table_name(flag);

    if (flag & flags) {
        CONFLATE_LOG(handle, DEBUG, "Table %s already exists", table_name);
    } else {
        if (db_do(handle, db, query)) {
            CONFLATE_LOG(handle, INFO, "Created table:  %s", table_name);
        } else {
            CONFLATE_LOG(handle, WARN, "DB error creating table:  %s", table_name);
            rv = false;
        }
    }

    return rv;
}

static bool initialize_db(conflate_handle_t *handle, sqlite3 *db)
{
    bool rv = true;
    char *errmsg = NULL;
    char* query = "SELECT name FROM sqlite_master "
        "WHERE type='table' "
        "ORDER BY name;";

    assert(db);

    struct table_mask_userdata udata = { 0, handle };

    if (sqlite3_exec(db, query, set_table_mask, &udata, &errmsg) != SQLITE_OK) {
        CONFLATE_LOG(handle, ERROR, "DB error:  %s", errmsg);
        sqlite3_free(errmsg);
    } else {
        rv &= maybe_create_table(handle, db, HAS_KEYS, udata.found, CREATE_KEYS);
        rv &= maybe_create_table(handle, db, HAS_VALUES, udata.found, CREATE_VALUES);
        rv &= maybe_create_table(handle, db, HAS_PRIVATE, udata.found, CREATE_PRIVATE);
    }

    return rv;
}

static int open_and_initialize_db(conflate_handle_t *handle,
                                  const char *filename,
                                  sqlite3 **db)
{
    int err = 0;

    if ((err = sqlite3_open(filename, &*db)) != SQLITE_OK) {
        return err;
    }

    if (!initialize_db(handle, *db)) {
        CONFLATE_LOG(handle, ERROR, "Error initializing tables");
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}

static int run_mod_steps(conflate_handle_t *handle, sqlite3 *db,
                         sqlite3_stmt *statement)
{
    int steps_run = 0, rc = 0;
    while ((rc = sqlite3_step(statement)) != SQLITE_DONE) {
        steps_run++;
        assert(steps_run < MAX_STEPS);
        CONFLATE_LOG(handle, DEBUG, "statement step result: %d", rc);
    }
    return sqlite3_changes(db);
}

bool save_kvpairs(conflate_handle_t *handle, kvpair_t* kvpair,
                  const char *filename)
{
    bool rv = false;
    int err = 0;
    sqlite3 *db = NULL;
    sqlite3_stmt *ins_keys = NULL, *ins_vals = NULL;
    const char* unused;

    if ((err = open_and_initialize_db(handle, filename, &db)) != SQLITE_OK) {
        goto finished;
    }

    assert(db != NULL);

    if (!db_do(handle, db, "begin transaction")) {
        goto finished;
    }

    /* Cleanup the existing stuff */

    if (!db_do(handle, db, "delete from keys")) {
        goto finished;
    }

    if (!db_do(handle, db, "delete from vals")) {
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
        int j = 0;
        sqlite_int64 key_id;

        sqlite3_clear_bindings(ins_keys);
        sqlite3_clear_bindings(ins_vals);

        sqlite3_bind_text(ins_keys, 1, kvpair->key, strlen(kvpair->key),
                          SQLITE_TRANSIENT);

        if (run_mod_steps(handle, db, ins_keys) != 1) {
            goto finished;
        }

        key_id = sqlite3_last_insert_rowid(db);

        for (j = 0; kvpair->values[j]; j++) {
            char* val = kvpair->values[j];

            sqlite3_bind_int64(ins_vals, 1, key_id);
            sqlite3_bind_text(ins_vals, 2, val, strlen(val), SQLITE_TRANSIENT);

            if (run_mod_steps(handle, db, ins_vals) != 1) {
                goto finished;
            }

            sqlite3_reset(ins_vals);
        }

        sqlite3_reset(ins_keys);

        kvpair = kvpair -> next;
    }

    if (!db_do(handle, db, "commit")) {
        goto finished;
    }

    rv = true;

 finished:
    if (sqlite3_errcode(db) != SQLITE_OK) {
        CONFLATE_LOG(handle, ERROR, "DB error %d:  %s",
                     sqlite3_errcode(db), sqlite3_errmsg(db));
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
    kvpair_t* pair = find_kvpair(*pairs, vals[0]);

    /* If such a list doesn't already exist, one will be assigned to you */
    if (!pair) {
        pair = mk_kvpair(vals[0], NULL);
        pair->next = *pairs;
        *pairs = pair;
    }

    add_kvpair_value(pair, vals[1]);

    return SQLITE_OK;
}

kvpair_t* load_kvpairs(conflate_handle_t *handle, const char *filename)
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
        CONFLATE_LOG(handle, ERROR, "DB error %d:  %s",
                     sqlite3_errcode(db), sqlite3_errmsg(db));
        if (errmsg) {
            CONFLATE_LOG(handle, ERROR, "  %s", errmsg);
            sqlite3_free(errmsg);
        }
    }

    sqlite3_close(db);

    return pairs;
}

bool conflate_save_private(conflate_handle_t *handle,
                           const char *k, const char *v, const char *filename)
{
    bool rv = false;
    int err = 0;
    sqlite3 *db = NULL;
    sqlite3_stmt *ins = NULL;
    const char *unused = NULL;

    if ((err = open_and_initialize_db(handle, filename, &db)) != SQLITE_OK) {
        goto finished;
    }

    assert(db != NULL);

    if ((err = sqlite3_prepare_v2(db, INS_PRIV, strlen(INS_PRIV),
                                  &ins, &unused)) != SQLITE_OK) {
        goto finished;
    }

    sqlite3_bind_text(ins, 1, k, strlen(k), SQLITE_TRANSIENT);
    sqlite3_bind_text(ins, 2, v, strlen(v), SQLITE_TRANSIENT);

    rv = run_mod_steps(handle, db, ins) == 1;

 finished:
    err = sqlite3_errcode(db);
    if (err != SQLITE_OK && err != SQLITE_DONE) {
        CONFLATE_LOG(handle, ERROR, "DB error %d:  %s",
                     sqlite3_errcode(db), sqlite3_errmsg(db));
    }

    if (ins) {
        sqlite3_finalize(ins);
    }

    sqlite3_close(db);

    return rv;
}

bool conflate_delete_private(conflate_handle_t *handle,
                             const char *k, const char *filename)
{
    return false;
}

char *conflate_get_private(conflate_handle_t *handle,
                           const char *k, const char *filename)
{
    return NULL;
}
