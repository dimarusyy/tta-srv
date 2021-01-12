#pragma once
#include <functional>

struct activity_t
{
    virtual ~activity_t() = default;
    
    virtual void start() = 0;
    virtual void stop() = 0;
};