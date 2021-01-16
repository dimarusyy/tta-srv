#pragma once
#include "activity.h"
#include "config/config.h"

struct kb_activity_t : activity_t
{
    kb_activity_t(const config_t::section_t& cfg);

    virtual void start() override;
    virtual void stop() override;

private:

};