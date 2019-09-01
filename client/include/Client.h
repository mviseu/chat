#pragma once
#include "HostPort.h"
#include <boost/asio.hpp>
#include <future>

namespace client {

class Client {
public:
  Client() : socket_(ioContext_) {}
  ~Client();
  auto Run(const HostPort &hostport) -> void;

private:
  auto Read() -> void;
  auto Write() -> void;
  boost::asio::io_context ioContext_;
  boost::asio::ip::tcp::socket socket_;
  std::mutex mtxSocket_;
  std::future<void> readFut_;
  std::future<void> writeFut_;
};

} // namespace client