#pragma once
#include "HostPort.h"
#include <boost/asio.hpp>

namespace client {
auto Connect(const HostPort &hostport, boost::asio::io_context &ioContext,
             boost::asio::ip::tcp::socket &socket) -> void;

} // namespace client