#pragma once
#include "HostPort.h"
#include <boost/asio.hpp>

namespace client {

class Client {
public:
  Client() : socket_(ioContext_) {}
  auto Run(const HostPort &hostport) -> void;

private:
  boost::asio::io_context ioContext_;
  boost::asio::ip::tcp::socket socket_;
};

} // namespace client