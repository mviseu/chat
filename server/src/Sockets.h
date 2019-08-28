#pragma once
#include <boost/asio.hpp>
#include <vector>

namespace server {

struct Sockets {
  Sockets(std::vector<boost::asio::ip::tcp::socket> sockets);
  std::vector<boost::asio::ip::tcp::socket> sockets;
  std::mutex mutexSockets;
};
} // namespace server