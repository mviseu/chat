#include "Client.h"
#include "Message.h"
#include <chrono>
#include <iostream>
#include <string>

namespace client {

namespace {

const std::string exitCommand("exit");

auto PrintMessage(const std::string &msg) -> void {
  std::cout << msg << std::endl;
}

auto PrintWelcomeMessage() -> void {
  std::ostringstream os;
  os << "Welcome to the Chat! To terminate the application at any stage type "
        "'exit'.";
  PrintMessage(os.str());
}

auto ToLower(const std::string &s) -> std::string {
  std::string lowerString(s);
  std::transform(lowerString.begin(), lowerString.end(), lowerString.begin(),
                 [](auto c) { return std::tolower(c); });
  return lowerString;
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

auto Client::DoWrite(const std::string &msg) -> bool {
  boost::system::error_code ec;
  std::unique_lock<std::mutex> lck(mtxSocket_);
  boost::asio::write(socket_, boost::asio::buffer(msg, msg.size()), ec);
  lck.unlock();
  if (ec) {
    throw boost::system::system_error(ec); // Some other error.
  }
  return true;
}

auto Client::ReadMessageSize() -> std::optional<int32_t> {
  // Read in msg header size that can have up to 4 characters representing
  // digits 0/9
  const auto msgSize = DoRead(nrDigitsInMsgHeader);
  return msgSize ? std::make_optional(std::stoi(*msgSize)) : std::nullopt;
}

auto Client::ReadMessageBody(int32_t msgSize) -> std::optional<std::string> {
  // Read in msg body
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

auto Client::Write() -> void {
  for (std::string message; std::getline(std::cin, message);) {
    if (exitCommand == ToLower(message)) {
      std::cout << "Exited" << std::endl;
      break;
    }
    const auto composedMsg = EncodeHeader(message);
    std::cout << "message: " << composedMsg << std::endl;
    const auto write = DoWrite(composedMsg);
    if (!write) {
      break;
    }
  }
}

// Run launches separate threads to read/write and returns after runFor duration
auto Client::Run(const HostPort &hostport) -> void {
  boost::asio::ip::tcp::resolver resolver(ioContext_);
  auto endpoints = resolver.resolve(hostport.host, hostport.port);
  boost::asio::connect(socket_, endpoints);
  PrintWelcomeMessage();
  readFut_ = std::async(std::launch::async, &Client::Read, this);
  writeFut_ = std::async(std::launch::async, &Client::Write, this);
  writeFut_.wait();
}

Client::~Client() {
  socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
  socket_.close();
  readFut_.get();
  writeFut_.get();
}

} // namespace client