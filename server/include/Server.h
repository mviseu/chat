#pragma once
#include <boost/asio.hpp>
#include <mutex>
#include <vector>

namespace server {

class Server {
public:
  Server() = default;
  auto Listen(int port) -> void;

private:
  boost::asio::io_context ioContext_;
  std::vector<boost::asio::ip::tcp::socket> clients_;
  std::mutex mtxClients_;
};

} // namespace server