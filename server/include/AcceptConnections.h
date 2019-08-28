#pragma once
#include "Sockets.h"
#include <boost/asio.hpp>

namespace server {
auto AcceptConnections(int port, boost::asio::io_context &ioContext,
                       Sockets sockets) -> void;
} // namespace server