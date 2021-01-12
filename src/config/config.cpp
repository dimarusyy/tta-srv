#include "config.h"
#include "logger.h"

config_t::config_t(const std::string& filename)
    : _config(toml::parse_file(filename))
{
    SPDLOG_INFO("config file {} parsed", filename);

    auto c = _config["keyboard"];
}