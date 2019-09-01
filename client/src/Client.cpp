#include "Client.h"
#include <iostream>

namespace client {

auto Client::Read() -> void {}

auto Client::Write() -> void {}

auto Client::Run(const HostPort &hostport) -> void {
  boost::asio::ip::tcp::resolver resolver(ioContext_);
  auto endpoints = resolver.resolve(hostport.host, hostport.port);
  boost::asio::connect(socket_, endpoints);
  readFut_ = std::async(&Client::Read, this);
  writeFut_ = std::async(&Client::Write, this);
}

Client::~Client() {
  std::unique_lock<std::mutex> lck(mtxSocket_);
  socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
  socket_.close();
  lck.unlock();
  readFut_.get();
  writeFut_.get();
}

} // namespace client