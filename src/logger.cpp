#include "logger.h"

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <fmt/core.h>

#include <date/date.h>

#if _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <chrono>
#endif

logger_t::lifetime::~lifetime()
{
    SPDLOG_INFO("logger shutdown");
    spdlog::drop("tta-service");
}

logger_t::lifetime::operator std::shared_ptr<spdlog::logger>() const
{
    return _logger;
}

spdlog::sink_ptr logger_t::lifetime::create_sink(console)
{
    return std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
}

spdlog::sink_ptr logger_t::lifetime::create_sink(file)
{
    const auto get_proc_id = []() {
#ifdef _WIN32
        return GetCurrentProcessId();
#else
        return ::getpid();
#endif
    };
    std::string log_file_name = fmt::format("logs/tta-servcie.{}.{}.log",
                                            get_proc_id(),
                                            date::format("%F_%H:%M:%OS", std::chrono::system_clock::now()));
    return std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file_name,
                                                                  1024 * 1024 * 5,
                                                                  5);
}
