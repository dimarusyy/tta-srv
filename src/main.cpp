#include "logger.h"
#include "config/config.h"
#include "storage/storage.h"

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


    return 0;
}