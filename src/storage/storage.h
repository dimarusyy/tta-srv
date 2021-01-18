#pragma once
#include "logger.h"
#include "model/exchange.h"

#if _WIN32
#define SQLITE_ORM_OMITS_CODECVT
#endif

#include <sqlite_orm/sqlite_orm.h>
namespace orm = sqlite_orm;

#include <vector>
#include <chrono>

class storage_t
{
    inline static const std::string DEFAULT_DB_NAME = "db.sqlite";
    inline static const std::string DEFAULT_TABLE_NAME = "events";

public:
    storage_t(config_t::section_t cfg)
        : _db(make_db(cfg["db_name"].value_or(DEFAULT_DB_NAME), cfg["table_name"].value_or(DEFAULT_TABLE_NAME)))
    {
        SPDLOG_INFO("using storage file [{}]", _db.filename());
        try
        {
            _db.sync_schema();
        }
        catch (const std::exception& ex)
        {
            SPDLOG_ERROR("exception [{}]", ex.what());
        }
    }

    auto iterate()
    {
        return _db.iterate<model::exchange_t>();
    }

    int insert(model::exchange_t&& msg)
    {
        int rc = 0;
        try
        {
            rc = _db.insert(std::move(msg));
        }
        catch (const std::exception& ex)
        {
            SPDLOG_ERROR("exception [{}]", ex.what());
        }
        return rc;
    }
 
    void remove_by_id(std::size_t id)
    {
        try
        {
            _db.remove<model::exchange_t>(id);
        }
        catch (const std::exception& ex)
        {
            SPDLOG_ERROR("exception [{}]", ex.what());
        }
    }
  
private:
    inline static auto make_db =
        [](const std::string& db_name, const std::string& table_name) {
        return orm::make_storage(db_name,
                                 orm::make_table(table_name,
                                                 orm::make_column("id", &model::exchange_t::id, orm::autoincrement(), orm::primary_key()),
                                                 orm::make_column("timestamp", &model::exchange_t::timestamp),
                                                 orm::make_column("event_data", &model::exchange_t::event_data)));
    };

    using db_type = std::invoke_result_t<decltype(make_db), const std::string&, const std::string&>;
    db_type _db;
};