#include "Client.h"
#include "Message.h"
#include <chrono>
#include <iostream>
#include <string>

namespace client {

namespace {

auto PrintMessage(const std::string &msg) -> void {
  std::cout << msg << std::endl;
}
} // namespace

auto Client::DoRead(int32_t nrBytes) -> std::optional<std::string> {
  std::string buf(nrBytes, '\0');
  boost::system::error_code ec;
  std::unique_lock<std::mutex> lck(mtxSocket_);
  boost::asio::read(socket_, boost::asio::buffer(buf, nrBytes), ec);
  lck.unlock();
  if (ec == boost::asio::error::eof) {
    return std::nullopt; // Connection closed cleanly by peer.
  }
  if (ec) {
    throw boost::system::system_error(ec); // Some other error.
  }
  return buf;
}

auto Client::ReadMessageSize() -> std::optional<int32_t> {
  // Read in msg header size that can have up to 4 characters representing
  // digits 0/9 + null termination
  const auto msgSize = DoRead(sizeOfMsgHeader);
  return msgSize ? std::make_optional(std::stoi(*msgSize)) : std::nullopt;
}

auto Client::ReadMessageBody(int32_t msgSize) -> std::optional<std::string> {
  // Read in msg body, size includes the null

  const auto msg = DoRead(msgSize);
  return msg ? std::make_optional(*msg) : std::nullopt;
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

// Run launches separate threads to read/write and returns after runFor duration
auto Client::Run(const HostPort &hostport, const std::chrono::seconds &runFor)
    -> void {
  boost::asio::ip::tcp::resolver resolver(ioContext_);
  auto endpoints = resolver.resolve(hostport.host, hostport.port);
  boost::asio::connect(socket_, endpoints);

  readFut_ = std::async(std::launch::async, &Client::Read, this);
  writeFut_ = std::async(std::launch::async, &Client::Write, this);

  readFut_.wait_for(runFor);
  writeFut_.wait_for(runFor);
}

Client::~Client() {
  socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
  socket_.close();
  readFut_.get();
  writeFut_.get();
}

} // namespace client