#pragma once

#include "traits.h"

#ifdef _WIN32
#define SPDLOG_FUNCTION __FUNCTION__
#else
#define SPDLOG_FUNCTION __PRETTY_FUNCTION__
#endif

#include <spdlog/spdlog.h>
#include <spdlog/async.h>

#include <memory>

struct logger_t final
{
    struct console
    {
    };
    struct file
    {
    };

    struct lifetime final
    {
        template <typename... Args>
        lifetime(Args... args)
        {
            spdlog::init_thread_pool(128, 1);
            spdlog::set_level(spdlog::level::debug);
            spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%P:%t] [%l] : [%!] - %v");
            spdlog::flush_every(std::chrono::seconds(1));

            spdlog::sink_ptr sinks[] = { create_sink(args)... };
            _logger = std::make_shared<spdlog::async_logger>("tta-service",
                                                             begin(sinks),
                                                             end(sinks),
                                                             spdlog::thread_pool(),
                                                             spdlog::async_overflow_policy::block);
            spdlog::set_default_logger(_logger);

            // print 1-st log message
            SPDLOG_INFO("logger initialized");
        }

        ~lifetime();

        operator std::shared_ptr<spdlog::logger>() const;

    private:
        static spdlog::sink_ptr create_sink(console);
        static spdlog::sink_ptr create_sink(file);

        std::shared_ptr<spdlog::logger> _logger;
    };

    template <typename... Args>
    static auto get(Args... args)
        -> typename std::enable_if<traits::contains<logger_t::console, Args...>::value ||
        traits::contains<logger_t::file, Args...>::value,
        std::shared_ptr<spdlog::logger>>::type
    {
        static logger_t::lifetime instance(args...);
        return instance.operator std::shared_ptr<spdlog::logger>();
    }
};

typedef std::shared_ptr<logger_t> logger_ptr_t;

