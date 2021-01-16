#pragma once

#include <string>
#include <toml++/toml.h>
#include <fstream>

struct config_t
{
    using section_t = toml::v2::node_view<const toml::v2::node>;

    config_t(const std::string& filename);

    section_t get_section(const std::string& section_name) const;
    
private:
    std::string _config_file_path;
    toml::parse_result _config;
};