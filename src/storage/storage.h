#pragma once
#include "model/exchange.h"

#if _WIN32
#define SQLITE_ORM_OMITS_CODECVT
#endif

#include <sqlite_orm/sqlite_orm.h>
namespace orm = sqlite_orm;

#include <type_traits>
#include <memory>

class storage_t
{
    inline static const std::string DEFAULT_DB_NAME = "db.sqlite";
    inline static const std::string DEFAULT_TABLE_NAME = "events";

public:
    storage_t(config_t::section_t cfg)
    {
        const std::string db_name = cfg["db_name"].value_or(DEFAULT_DB_NAME);
        const std::string table_name = cfg["table_name"].value_or(DEFAULT_TABLE_NAME);
        
        //_db = std::make_unique<value_type>(db_name, table_name);
    }

private:
    inline static auto make_id_column =
        []() { return orm::make_column("id", &message::exchange_t::id, orm::autoincrement()); };

    inline static auto make_table =
        [](const std::string& table_name) { return orm::make_table("table_name", make_id_column()); };

    inline static auto make_db =
        [](const std::string& db_name, const std::string& table_name) { return orm::make_storage(db_name, make_table(table_name)); };

    using value_type = std::invoke_result<decltype(make_db), const std::string&, const std::string&> ::type;
    std::unique_ptr<value_type> _db;
};