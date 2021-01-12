#pragma once
#include "activity.h"

struct kb_activity_t : activity_t
{
    kb_activity_t();

    virtual void start() override;
    virtual void stop() override;

private:

};