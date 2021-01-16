#include "config.h"
#include "logger.h"

#include <boost/filesystem.hpp>

config_t::config_t(const std::string& filename)
    : _config_file_path((boost::filesystem::current_path() / filename).string())
    , _config(toml::parse_file(_config_file_path))
{
    SPDLOG_INFO("Trying config file : {}", _config_file_path);
}

config_t::section_t 
config_t::get_section(const std::string& section_name) const
{
    return _config[section_name];
}
