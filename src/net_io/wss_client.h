#pragma once
#include "net.h"
#include "model/exchange.h"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <deque>

struct client_config_t
{
    std::string host;
    std::string port;
};

struct exchange_t;

class wss_client_t
    : public std::enable_shared_from_this<wss_client_t>
{
public:
    wss_client_t(net::io_context& io);
    ~wss_client_t();

    virtual void connect(const std::string& host,
                         const std::string& port);

    virtual void send(message::exchange_t&& msg);

    void send(std::string&& data);

    net::executor get_executor();

    void shutdown();

protected:
    void on_read(boost::beast::error_code ec, std::size_t size);

private:
    bool _stop{ false };

    net::io_context& _io;
    tcp::resolver _resolver;
    boost::beast::websocket::stream<boost::beast::tcp_stream> _ws;
    net::basic_waitable_timer<std::chrono::steady_clock> _connect_timer;

    std::deque<std::string> _cache;
    boost::beast::flat_buffer _buffer;
};

typedef std::shared_ptr<wss_client_t> wss_client_ptr_t;
