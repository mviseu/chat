#include "Client.h"
#include "Message.h"
#include <chrono>
#include <iostream>
#include <string>

namespace client {

namespace {

const std::string exitCommand("exit");

auto IsReady(const std::future<void> &fut) -> bool {
  return fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

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
  for (;;) {
    // exit has been pressed
    if (IsReady(writeFut_)) {
      std::cout << "write future is ready" << std::endl;
      return std::nullopt; // Exit has been typed by this client
    }
    std::unique_lock<std::mutex> lck(mtxSocket_);
    boost::asio::socket_base::bytes_readable command(true);
    socket_.io_control(command);
    auto nrBytesReadable = command.get();
    if (nrBytesReadable == static_cast<size_t>(nrBytes)) {
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
    lck.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

auto Client::DoWrite(const std::string &msg) -> bool {
  boost::system::error_code ec;
  std::cout << "write started before lock" << std::endl;
  std::unique_lock<std::mutex> lck(mtxSocket_);
  std::cout << "write started after lock" << std::endl;
  boost::asio::write(socket_, boost::asio::buffer(msg, msg.size()), ec);
  lck.unlock();
  std::cout << "write done" << std::endl;
  if (ec) {
    throw boost::system::system_error(ec); // Some other error.
  }
  return true;
}

auto Client::ReadMessageSize() -> std::optional<int32_t> {
  // Read in msg header size that can have up to 4 characters representing
  // digits 0/9
  const auto msgSize = DoRead(nrDigitsInMsgHeader);
  return (msgSize != std::nullopt) ? std::make_optional(std::stoi(*msgSize))
                                   : std::nullopt;
}

auto Client::ReadMessageBody(int32_t msgSize) -> std::optional<std::string> {
  // Read in msg body
  return DoRead(msgSize);
}

auto Client::Read() -> void {
  for (;;) {
    const auto sizeMsg = ReadMessageSize();
    if (sizeMsg == std::nullopt) {
      std::cout << "going to break in Cient::Read" << std::endl;
      break;
    }
    std::cout << "after size" << std::endl;
    const auto msg = ReadMessageBody(*sizeMsg);
    if (msg == std::nullopt) {
      break;
    }
    PrintMessage(*msg);
  }
  std::cout << "Exit read" << std::endl;
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
  std::cout << "complete run" << std::endl;
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