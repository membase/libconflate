#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "conflate.h"
#include "conflate_internal.h"
#include "conflate_convenience.h"

#ifndef CONFLATE_NO_SQLITE

#include <sqlite3.h>

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
#define DEL_PRIV "delete from private where key = ?"
#define SEL_PRIV "select value from private where key = ?"

#define LOAD_KVPAIRS "select k.name, v.value " \
    "from keys k join vals v on (k.id = v.key_id)"

/* safety-net */
#define MAX_STEPS 1024

static int run_mod_steps(conflate_handle_t *handle, sqlite3 *db,
                         sqlite3_stmt *statement)
    __libconflate_gcc_attribute__ ((warn_unused_result, nonnull(1, 2, 3)));

/** \private */
struct table_mask_userdata {
    int found;
    conflate_handle_t *handle;
};

static int set_table_mask(void* arg, int n, char **vals, char **cols)
{
    (void)cols;
    struct table_mask_userdata *udata = (struct table_mask_userdata*)arg;
    assert(n == 1);

    if (strcmp(vals[0], "keys") == 0) {
        udata->found |= HAS_KEYS;
    } else if(strcmp(vals[0], "vals") == 0) {
        udata->found |= HAS_VALUES;
    } else if(strcmp(vals[0], "private") == 0) {
        udata->found |= HAS_PRIVATE;
    } else {
        CONFLATE_LOG(udata->handle, LOG_LVL_WARN, "Unknown table:  %s", vals[0]);
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
        fprintf(stderr, "Unhandled flag:  %x\n", flag);
        abort();
    }
}

static bool db_do(conflate_handle_t *handle, sqlite3 *db, const char* query)
{
    char* errmsg = NULL;
    bool rv = true;

    if (sqlite3_exec(db, query, NULL, NULL, &errmsg) != SQLITE_OK) {
        CONFLATE_LOG(handle, LOG_LVL_ERROR, "DB Error:  %s\n%s", errmsg, query);
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
        CONFLATE_LOG(handle, LOG_LVL_DEBUG, "Table %s already exists", table_name);
    } else {
        if (db_do(handle, db, query)) {
            CONFLATE_LOG(handle, LOG_LVL_INFO, "Created table:  %s", table_name);
        } else {
            CONFLATE_LOG(handle, LOG_LVL_WARN, "DB error creating table:  %s", table_name);
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
        CONFLATE_LOG(handle, LOG_LVL_ERROR, "DB error:  %s", errmsg);
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
        CONFLATE_LOG(handle, LOG_LVL_ERROR, "Error initializing tables");
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
        CONFLATE_LOG(handle, LOG_LVL_DEBUG, "statement step result: %d", rc);
    }
    return sqlite3_changes(db);
}

bool save_kvpairs(conflate_handle_t *handle, kvpair_t* kvpair,
                  const char *filename)
{
    bool rv = false;
    sqlite3 *db = NULL;
    sqlite3_stmt *ins_keys = NULL, *ins_vals = NULL;

    if ( open_and_initialize_db(handle, filename, &db) != SQLITE_OK) {
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
                           &ins_keys, NULL) != SQLITE_OK) {
        goto finished;
    }
    /* Add the values */
    if (sqlite3_prepare_v2(db, INS_VALS, strlen(INS_VALS),
                           &ins_vals, NULL) != SQLITE_OK) {
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
        CONFLATE_LOG(handle, LOG_LVL_ERROR, "DB error %d:  %s",
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
    (void)n;
    (void)cols;
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
        CONFLATE_LOG(handle, LOG_LVL_ERROR, "DB error %d:  %s",
                     sqlite3_errcode(db), sqlite3_errmsg(db));
        if (errmsg) {
            CONFLATE_LOG(handle, LOG_LVL_ERROR, "  %s", errmsg);
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

    if ((err = open_and_initialize_db(handle, filename, &db)) != SQLITE_OK) {
        goto finished;
    }

    assert(db != NULL);

    if ((err = sqlite3_prepare_v2(db, INS_PRIV, strlen(INS_PRIV),
                                  &ins, NULL)) != SQLITE_OK) {
        goto finished;
    }

    sqlite3_bind_text(ins, 1, k, strlen(k), SQLITE_TRANSIENT);
    sqlite3_bind_text(ins, 2, v, strlen(v), SQLITE_TRANSIENT);

    rv = run_mod_steps(handle, db, ins) == 1;

 finished:
    err = sqlite3_errcode(db);
    if (err != SQLITE_OK && err != SQLITE_DONE) {
        CONFLATE_LOG(handle, LOG_LVL_ERROR, "DB error %d:  %s",
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
    bool rv = false;
    int err = 0, deleted = 0;
    sqlite3 *db = NULL;
    sqlite3_stmt *del = NULL;

    if ((err = open_and_initialize_db(handle, filename, &db)) != SQLITE_OK) {
        goto finished;
    }

    assert(db != NULL);

    if ((err = sqlite3_prepare_v2(db, DEL_PRIV, strlen(DEL_PRIV),
                                  &del, NULL)) != SQLITE_OK) {
        goto finished;
    }

    sqlite3_bind_text(del, 1, k, strlen(k), SQLITE_TRANSIENT);

    deleted = run_mod_steps(handle, db, del);
    CONFLATE_LOG(handle, LOG_LVL_DEBUG, "Removed %d records", deleted);
    rv = deleted >= 0;

 finished:
    err = sqlite3_errcode(db);
    if (err != SQLITE_OK && err != SQLITE_DONE) {
        CONFLATE_LOG(handle, LOG_LVL_ERROR, "DB error %d:  %s",
                     sqlite3_errcode(db), sqlite3_errmsg(db));
    }

    if (del) {
        sqlite3_finalize(del);
    }

    sqlite3_close(db);

    return rv;
}

char *conflate_get_private(conflate_handle_t *handle,
                           const char *k, const char *filename)
{
    char *errmsg = NULL;
    sqlite3 *db = NULL;
    sqlite3_stmt *get = NULL;
    char *rv = NULL;
    bool done = false;
    int steps_run = 0, err = 0;

    if (sqlite3_open(filename, &db) != SQLITE_OK) {
        goto finished;
    }

    if (sqlite3_prepare_v2(db, SEL_PRIV, strlen(SEL_PRIV),
                           &get, NULL) != SQLITE_OK) {
        goto finished;
    }

    sqlite3_bind_text(get, 1, k, strlen(k), SQLITE_TRANSIENT);

    while (!done) {
        switch(sqlite3_step(get)) {
        case SQLITE_BUSY:
            CONFLATE_LOG(handle, LOG_LVL_INFO, "DB was busy, retrying...\n");
            break;
        case SQLITE_ROW:
            assert(rv == NULL);
            rv = safe_strdup((char*)sqlite3_column_text(get, 0));
            CONFLATE_LOG(handle, LOG_LVL_DEBUG, "Retrieved value for ``%s'':  ``%s''",
                         k, rv);
            break;
        case SQLITE_DONE:
            done = true;
            break;
        }
        ++steps_run;
        assert(steps_run < MAX_STEPS);
    }

 finished:
    err = sqlite3_errcode(db);
    if (err != SQLITE_OK && err != SQLITE_DONE) {
        CONFLATE_LOG(handle, LOG_LVL_ERROR, "DB error %d:  %s",
                     sqlite3_errcode(db), sqlite3_errmsg(db));
        if (errmsg) {
            CONFLATE_LOG(handle, LOG_LVL_ERROR, "  %s", errmsg);
            sqlite3_free(errmsg);
        }
    }

    if (get) {
        sqlite3_finalize(get);
    }

    sqlite3_close(db);

    return rv;
}

#else // !CONFLATE_NO_SQLITE

kvpair_t* load_kvpairs(conflate_handle_t *handle, const char *filename)
{
    (void)handle;
    (void)filename;
    return NULL;
}

bool save_kvpairs(conflate_handle_t *handle, kvpair_t* kvpair,
                  const char *filename)
{
    (void)handle;
    (void)kvpair;
    (void)filename;
    return true;
}

bool conflate_delete_private(conflate_handle_t *handle,
                             const char *k, const char *filename)
{
    (void)handle;
    (void)k;
    (void)filename;
    return true;
}

bool conflate_save_private(conflate_handle_t *handle,
                           const char *k, const char *v, const char *filename)
{
    (void)handle;
    (void)k;
    (void)v;
    (void)filename;
    return true;
}

char *conflate_get_private(conflate_handle_t *handle,
                           const char *k, const char *filename)
{
    (void)handle;
    (void)k;
    (void)filename;
    return NULL;
}

#endif // !CONFLATE_NO_SQLITE
