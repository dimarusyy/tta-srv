#pragma once
#include "logger.h"
#include "model/exchange.h"

#if _WIN32
#define SQLITE_ORM_OMITS_CODECVT
#endif

#include <SQLiteCpp/SQLiteCpp.h>

#if 0
#include <sqlite_orm/sqlite_orm.h>
namespace orm = sqlite_orm;
#endif


class storage_t
{
    inline static const std::string DEFAULT_DB_NAME = "db.sqlite";
    inline static const std::string DEFAULT_TABLE_NAME = "events";

public:
    storage_t(config_t::section_t cfg)
        : _db(cfg["db_name"].value_or(DEFAULT_DB_NAME), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE | SQLite::OPEN_FULLMUTEX)
    {
        SPDLOG_INFO("using storage file [{}]", _db.getFilename());
        try
        {
            const std::string table_name = cfg["table_name"].value_or(DEFAULT_TABLE_NAME);
            auto rc = _db.exec("CREATE TABLE IF NOT EXISTS [" + table_name + "] ("
                               "[id] INTEGER PRIMARY KEY NOT NULL,"
                               "[timestamp] DATETIME NOT NULL,"
                               "[event_data] TEXT)");
        }
        catch (const std::exception& ex)
        {
            SPDLOG_ERROR("exception [{}]", ex.what());
        }
    }

private:
#if 0
    inline static auto make_id_column =
        []() { return orm::make_column("id", &model::exchange_t::id, orm::autoincrement(), orm::primary_key()); };

    inline static auto make_timestamp_column =
        []() { return orm::make_column("timestamp", &model::exchange_t::timestamp); };

    inline static auto make_event_data_column =
        []() { return orm::make_column("event_data", &model::exchange_t::event_data); };

    inline static auto make_table =
        [](const std::string& table_name) { return orm::make_table(table_name, make_id_column(), make_timestamp_column(), make_event_data_column); };

    inline static auto make_db =
        [](const std::string& db_name, const std::string& table_name) { return orm::make_storage(db_name, make_table(table_name)); };

    using db_type = std::invoke_result_t<decltype(make_db), const std::string&, const std::string&>;
#endif    
    SQLite::Database _db;
};