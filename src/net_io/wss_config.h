#pragma once
#include <chrono>

namespace wss_config
{
    constexpr auto connect_timeout()
    {
        return std::chrono::seconds(30);
    }

    constexpr auto send_retry_timeout()
    {
        return std::chrono::milliseconds(500);
    }
} // namespace wss_config
