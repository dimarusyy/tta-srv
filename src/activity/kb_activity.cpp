#include "kb_activity.h"
#include "config/config.h"
#include "logger.h"

kb_activity_t::kb_activity_t(const config_t::section_t& cfg)
{

}

void kb_activity_t::start()
{

}

void kb_activity_t::stop()
{

}

void kb_activity_t::set_on_data(std::function<void(boost::json::value)> handler)
{
    _on_data_handler = handler;
}