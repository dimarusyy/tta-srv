#include <boost/json/src.hpp>
#include "net_io/wss_client.h"

#include "logger.h"
#include "net_io/wss_config.h"
#include "model/exchange.h"

#include <boost/asio/strand.hpp>
#include <boost/asio/spawn.hpp>

//
namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace websocket = boost::beast::websocket;
//

wss_client_t::wss_client_t(net::io_context& io)
    : _resolver(net::make_strand(io))
    , _ws(net::make_strand(io))
{
}

void wss_client_t::connect(const std::string& host,
                           const std::string& port,
                           post_op_t handler)
{
    net::spawn(
        _ws.get_executor(),
        [host, port, this, self = shared_from_this(), handler = std::move(handler)](net::yield_context yield) {
        if (_state == state::connecting)
        {
            SPDLOG_INFO("ignoring as in [connecting] state");
            return;
        }
        _state = state::connecting;

        SPDLOG_DEBUG("connecting to {}:{}", host, port);
        beast::error_code ec;

        tcp::resolver resolver(_ws.get_executor());
        // Look up the domain name
        auto const results = resolver.async_resolve(host, port, yield[ec]);
        if (ec)
        {
            SPDLOG_CRITICAL("Failed to resolve : {}", ec.message());
            _state = state::disconnected;
            handler(ec);
            return;
        }

        SPDLOG_INFO("resolved {}", host);
        beast::get_lowest_layer(_ws).expires_after(wss_config::connect_timeout());

        // try to connect...
        auto ep = beast::get_lowest_layer(_ws).async_connect(results, yield[ec]);
        if (ec)
        {
            SPDLOG_CRITICAL("Failed to connect, error : {}", ec.message());
            _state = state::disconnected;
            handler(ec);
            return;
        }
        SPDLOG_INFO("connected to {}, endpoint port {}", host, ep.port());

        // no timeout more
        beast::get_lowest_layer(_ws).expires_never();

        auto url = host + ':' + std::to_string(ep.port());
        SPDLOG_INFO("initiating wss handshake to url=[{}]", url);

        // set suggested timeout settings for the websocket
        _ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

        // set a decorator to change the User-Agent of the handshake
        _ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req) {
                req.set(http::field::user_agent,
                        std::string(BOOST_BEAST_VERSION_STRING) + " tta-srv-client");
            }));

        // websocket handshake
        _ws.async_handshake(url, "/", yield[ec]);
        if (ec)
        {
            SPDLOG_CRITICAL("WSS handshake to connect : {}", ec.message());
            _state = state::disconnected;
            handler(ec);
            return;
        }

        SPDLOG_DEBUG("wss handshake succeded");

        // update state
        _state = state::connected;
        handler(ec);

        // read the message
        _ws.async_read(
            _buffer,
            beast::bind_front_handler(&wss_client_t::on_read, shared_from_this()));
    });
}

void wss_client_t::send(model::exchange_t&& msg, post_op_t handler)
{
    const auto jv = json::value_from(msg);
    send(std::move(json::serialize(jv)), handler);
}

void wss_client_t::send(std::string&& data, post_op_t handler)
{
    net::spawn(
        _ws.get_executor(),
        [this, self = shared_from_this(), data = std::move(data), handler](net::yield_context yield) mutable {
        if (_stop)
        {
            SPDLOG_INFO("skipping send as shutting down");
            return;
        }

        beast::error_code ec;
        if (self->_state == state::connected)
        {
            _ws.async_write(net::buffer(data), yield[ec]);
            handler(ec);
            if (!ec)
            {
                return;
            }
            // change state
            self->_state = state::disconnected;
        }
        else
        {
            handler(boost::asio::error::try_again);
        }

        // async_write failed, wait asynchronously until next attempt
        SPDLOG_ERROR("wss async_write failed, error: {}", ec.message());
    });
}

void wss_client_t::shutdown()
{
    _ws.async_close({}, [self = shared_from_this()](beast::error_code ec) {
        SPDLOG_INFO("async_close, ec: [{}]", ec.message());
    });

    net::post(_ws.get_executor(), [this, self = shared_from_this()]() {
        _stop = true;
    });
}

void wss_client_t::on_read(beast::error_code ec, std::size_t size)
{
    if (ec)
    {
        SPDLOG_CRITICAL("on_read failed : {}", ec.message());
        return;
    }

    SPDLOG_TRACE("read {} bytes", size);

    // [TODO : handle message from server]

    // reset buffer
    _buffer.consume(_buffer.size());

    //continue reading...
    _ws.async_read(
        _buffer,
        beast::bind_front_handler(&wss_client_t::on_read, shared_from_this()));
}
