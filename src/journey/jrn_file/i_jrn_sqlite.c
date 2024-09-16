#include "jrn_sqlite.h"
#include "jrn_internal.h"
#include "jrn_record.h"
#include "jrn_memory.h"

// FWD

extern i_jrn_settings_t i_jrn_settings;

extern const char *
jrn_get_log_type(jrn_level_t log_type);
/*

    SQL_STRUCTURE

    Record
    *************************************

        id:         |       primary key
        uuid:       |       varchar
        type:       |       varchar
        time:       |       integer
        user:       |       varchar
        message:    |       varchar
        file:       |       varchar
        line:       |       integer

    *************************************

    Stacktrace
    *************************************
        id:         |       primary key
        record_uuid:|       varchar
        line:       |       varchar

    *************************************

    Memory tick
    *************************************
        id:         |       primary key
        record_uuid:|       varchar
        values:     |       varchar

    *************************************
*/

sqlite3 *jrn_sqlite_open_db(const char *db_file)
{
    sqlite3 *db = NULL;
    sqlite3_open(db_file, &db);
    if (db == NULL)
    {
        jrn_internal_signal(JRN_ERR, "Failed to open SQLite");
    }
    jrn_internal_signal(JRN_OK, "SQlite ok");

    const char q[JRN_BUF_512] =
        "create table if not exists record ("
        "id integer primary key,"
        "uuid text not null,"
        "type text not null,"
        "time integer not null,"
        "user text not null,"
        "message text not null,"
        "file text not null,"
        "line integer not null"
        ");"
        "create table if not exists stacktrace ("
        "id integer primary key,"
        "record_uuid text not null,"
        "line text not null"
        ");"
        "create table if not exists mem ("
        "id integer primary key,"
        "record_uuid text not null,"
        "data text not null"
        ");";

    if (!(sqlite3_exec(db, q, NULL, NULL, NULL) == SQLITE_OK))
    {

        jrn_internal_signal(JRN_ERR, (const char *)sqlite3_errmsg(db));
    }
    else
    {
        jrn_internal_signal(JRN_OK, "SQLite initialized ok");
    }

    return db;
}

static inline void i_jrn_sqlite_quotes_wrap(const char *q, const char *val, bool add_comma)
{
    str_n_cat(q, "'");
    str_n_cat(q, val);
    if (add_comma)
    {
        str_n_cat(q, "', ");
    }
    else
    {
        str_n_cat(q, "' ");
    }
}

static inline void
i_jrn_sqlite_write_memory(char *sqlite_insert, char *uuid)
{
}

static inline void
i_jrn_sqlite_write_stacktrace(char *sqlite_insert, char *uuid)
{
}

void i_jrn_sqlite_create_entry(jrn_record_t *record)
{
    sqlite3 *db = i_jrn_settings.get_sqlite();
    if (db == NULL)
    {
        jrn_internal_signal(JRN_INFO_DB, "SQLite is not set, skipping record");
        return;
    }

    const char *level_str =
        jrn_get_log_type(record->debug_level);

    char record_uuid[JRN_BUF_64] = "";
    jrn_generate_uuid(record_uuid);

    char q[JRN_BUF] = "";

    str_n_cat(q, "insert into record (uuid, type, time, user, message, file, line) values (");
    i_jrn_sqlite_quotes_wrap(q, record_uuid, true);
    i_jrn_sqlite_quotes_wrap(q, level_str, true);
    str_n_cat(q, "datetime('now'), ");
    i_jrn_sqlite_quotes_wrap(q, record->user, true);
    i_jrn_sqlite_quotes_wrap(q, record->message, true);
    i_jrn_sqlite_quotes_wrap(q, record->__file__, true);
    str_n_cat(q, record->__line__);
    str_n_cat(q, "); ");

    if (record->stacktrace != NULL)
    {
        for (size_t i = 0; i != record->stacktrace_sz; i++)
        {
            str_n_cat(q, "insert into stacktrace (record_uuid, line) values (");
            i_jrn_sqlite_quotes_wrap(q, record_uuid, true);
            i_jrn_sqlite_quotes_wrap(q, record->stacktrace[i], false);
            str_n_cat(q, "); ");
        }
    }

    //////// MEM END

    jrn_memory_t *mem = i_jrn_settings.get_memory();

    if (mem == NULL)
    {
        jrn_internal_signal(JRN_OK, "Memory is not initialized");
        return;
    }

    char graph_buf[JRN_BUF] = "["; //making text visible as json
    jrn_memory_fill_graph(mem, graph_buf);
    str_n_cat(graph_buf, "]");

    str_n_cat(q, "insert into mem (record_uuid, data) values (");
    i_jrn_sqlite_quotes_wrap(q, record_uuid, true);
    i_jrn_sqlite_quotes_wrap(q, graph_buf, false);
    str_n_cat(q, "); ");

    //////// MEM END

    if (!(sqlite3_exec(db, q, NULL, NULL, NULL) == SQLITE_OK))
    {

        jrn_internal_signal(JRN_ERR, (const char *)sqlite3_errmsg(db));
    }
}
