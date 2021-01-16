#pragma once

#include "json.h"

#include <string>
#include <vector>

namespace model
{
    struct exchange_t
    {
        std::uint64_t id;
        std::size_t timestamp;
        std::string event_data;
    };

    inline void tag_invoke(
        json::value_from_tag,
        json::value& jv,
        exchange_t const& msg)
    {
        jv = {
            {"id", msg.id},
            {"timestamp", msg.timestamp},
            {"event_data", msg.event_data},
        };
    }

    inline exchange_t tag_invoke(json::value_to_tag<exchange_t>, const json::value& jv)
    {
        const json::object& obj = jv.as_object();
        return exchange_t{
            obj.at("id").as_uint64(),
            obj.at("timestamp").as_uint64(),
            std::string(json::serialize(obj.at("event_data")))
        };
    }
} // namespace message
