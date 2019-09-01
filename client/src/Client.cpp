#include "Client.h"
#include <iostream>
#include <string>

namespace client {

namespace {

constexpr int nrDigitsInMsgHeader = 4;
constexpr int sizeOfMsgHeader = nrDigitsInMsgHeader + 1;

auto PrintMessage(const std::string &msg) -> void {
  std::cout << msg << std::endl;
}
} // namespace

auto Client::ReadMessageSize() -> std::optional<int32_t> {
  // Read in msg header size that can have up to 4 characters representing
  // digits 0/9 + null termination
  std::string buf(sizeOfMsgHeader, '\0');
  boost::system::error_code ec;
  std::unique_lock<std::mutex> lck(mtxSocket_);
  boost::asio::read(socket_, boost::asio::buffer(buf, sizeOfMsgHeader), ec);
  lck.unlock();
  if (ec == boost::asio::error::eof) {
    return std::nullopt; // Connection closed cleanly by peer.
  }
  if (ec) {
    throw boost::system::system_error(ec); // Some other error.
  }
  return std::stoi(buf);
}

// message size includes the null
auto Client::ReadMessageBody(int64_t msgSize) -> std::optional<std::string> {
  // Read in msg body, size includes the null
  std::string buf(msgSize, '\0');
  boost::system::error_code ec;
  std::unique_lock<std::mutex> lck(mtxSocket_);
  boost::asio::read(socket_, boost::asio::buffer(buf, msgSize), ec);
  lck.unlock();
  if (ec == boost::asio::error::eof) {
    return std::nullopt; // Connection closed cleanly by peer.
  }
  if (ec) {
    throw boost::system::system_error(ec); // Some other error.
  }
  return buf;
}

auto Client::Read() -> void {
  for (;;) {
    const auto sizeMsg = ReadMessageSize();
    if (!sizeMsg) {
      break;
    }
    const auto msg = ReadMessageBody(*sizeMsg);
    if (!msg) {
      break;
    }
    PrintMessage(*msg);
  }
}

auto Client::Write() -> void {}

// Run launches separate threads to read/write and returns immediately
auto Client::Run(const HostPort &hostport) -> void {
  boost::asio::ip::tcp::resolver resolver(ioContext_);
  auto endpoints = resolver.resolve(hostport.host, hostport.port);
  boost::asio::connect(socket_, endpoints);
  readFut_ = std::async(std::launch::async, &Client::Read, this);
  writeFut_ = std::async(std::launch::async, &Client::Write, this);
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