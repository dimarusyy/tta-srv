#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>

#include "logger.h"
#include "config/config.h"
#include "storage/storage.h"
#include <activity/activity_manager.h>

#include <iostream>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: tta-service <config file>\n";
        return 1;
    }

    // initiate logger
    logger_t::get(logger_t::console{});

    // read config
    config_t cfg(argv[1]);

    //init storage
    storage_t storage(cfg.get_section("storage"));

    boost::asio::io_context io;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work(io.get_executor());

    // create activity_manager
    auto am = std::make_shared<activity_manager_t>(io, cfg);

    // handle signals
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);
    signals.async_wait([&work, am](const boost::system::error_code& ec,
                                   int signal_number)
                       {
                           if (!ec)
                           {
                               SPDLOG_INFO("resetting worker");
                               am->shutdown();
                               work.reset();
                           }
                           else
                           {
                               SPDLOG_CRITICAL("exception: {}", ec.message());
                           }
                       });

    // start
    am->startup();

    // run io_context....
    try
    {
        io.run();
    }
    catch (const std::exception& ex)
    {
        SPDLOG_CRITICAL("exception io::run() : [{}]", ex.what());
        return 1;
    }

    SPDLOG_INFO("exiting with 0");

    return 0;
}