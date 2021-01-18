#pragma once
#include "net.h"
#include "model/exchange.h"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <functional>

class wss_client_t
    : public std::enable_shared_from_this<wss_client_t>
{
    enum class state
    {
        connecting,
        connected,
        disconnected
    };

public:
    using post_op_t = std::function<void(const boost::system::error_code& ec)>;

    wss_client_t(net::io_context& io);

    void connect(const std::string& host,
                 const std::string& port,
                 post_op_t handler = {});
    void shutdown();

    virtual void send(model::exchange_t&& msg, post_op_t handler = {});
    virtual void send(std::string&& data, post_op_t handler = {});

protected:
    virtual void on_read(boost::beast::error_code ec, std::size_t size);

private:
    bool _stop{ false };
    state _state{ state::disconnected };

    tcp::resolver _resolver;
    boost::beast::websocket::stream<boost::beast::tcp_stream> _ws;

    boost::beast::flat_buffer _buffer;
};

typedef std::shared_ptr<wss_client_t> wss_client_ptr_t;
