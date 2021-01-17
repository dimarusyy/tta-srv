#pragma once
#include "model/json.h"
#include <functional>

struct activity_t
{
    virtual ~activity_t() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void set_on_data(std::function<void(boost::json::value)>) = 0;
};