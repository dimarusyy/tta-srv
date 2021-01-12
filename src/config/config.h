#pragma once

#include <string>
#include <toml++/toml.h>
#include <fstream>

struct config_t
{
    config_t(const std::string& filename);
    
private:
    toml::parse_result _config;
};