#pragma once
#include "net_io/net.h"
#include "net_io/wss_client.h"

#include "activity.h"
#include "config/config.h"

#include "storage/storage.h"

#include <functional>
#include <map>
#include <memory>
#include <chrono>

class activity_manager_t : 
    public std::enable_shared_from_this<activity_manager_t>
{
    inline static const std::chrono::minutes PUSH_TIMEOUT{ 10 };
    enum class state
    {
        sending,
        waiting
    };

public:
    activity_manager_t(net::io_context& io, const config_t& cfg);
    ~activity_manager_t();

    void startup();
    void shutdown();

    void start_activity(const std::string& activity_name);
    void stop_activity(const std::string& activity_name);

protected:
    void send_cached_events();

private:
    void connect_impl();
    void update(const std::string& activity_name,
                std::function<void(activity_t*)> handler);

    bool _stop{ false };
    state _state{ state::waiting };

    net::io_context& _io;
    const config_t& _cfg;

    // timer for 10-min delay
    net::basic_waitable_timer<std::chrono::steady_clock> _push_timer;

    // wss_client
    wss_client_ptr_t _wss_client;

    // storage
    storage_t _storage;

    std::map<std::string, std::unique_ptr<activity_t>> _activities;
};

typedef std::shared_ptr<activity_manager_t> activity_manager_ptr_t;