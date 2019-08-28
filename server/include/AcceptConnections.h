#pragma once
#include <boost/asio.hpp>
#include <vector>

namespace server {
auto AcceptConnections(int port, boost::asio::io_context &ioContext,
                       std::vector<boost::asio::ip::tcp::socket> &clients,
                       std::mutex &mutClients) -> void;
} // namespace server