#pragma once
#include "HostPort.h"
#include <boost/asio.hpp>
#include <future>
#include <optional>

namespace client {

class Client {
public:
  Client() : socket_(ioContext_) {}
  ~Client();
  auto Run(const HostPort &hostport) -> void;

private:
  auto Exit() -> void;
  auto DoRead(int32_t nrBytes) -> std::optional<std::string>;
  auto DoWrite(const std::string &) -> bool;
  auto Read() -> void;
  auto Write() -> void;
  auto ReadMessageSize() -> std::optional<int32_t>;
  auto ReadMessageBody(int32_t msgSize) -> std::optional<std::string>;
  auto CanWeWriteToServer() -> void;
  boost::asio::io_context ioContext_;
  boost::asio::ip::tcp::socket socket_;
  std::mutex mtxSocket_;
  std::future<void> readFut_;
  std::future<void> writeFut_;
  std::future<void> isDeadFut_;
};

} // namespace client