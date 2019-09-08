#include "Client.h"
#include "GetLineAsync.h"
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

auto QuickSleep() -> void {
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

auto LongSleep() -> void {
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

} // namespace

auto Client::DoRead(int32_t nrBytes) -> std::optional<std::string> {
  std::string buf(nrBytes, '\0');
  boost::system::error_code ec;
  for (;;) {
    // exit has been pressed
    if (IsReady(isDeadFut_)) {
      return std::nullopt; // Exit has been typed by this client
    }
    std::unique_lock<std::mutex> lck(mtxSocket_);
    boost::asio::socket_base::bytes_readable command(true);
    socket_.io_control(command);
    auto nrBytesReadable = command.get();
    if (nrBytesReadable >= static_cast<size_t>(nrBytes)) {
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
    QuickSleep();
  }
}

auto Client::DoWrite(const std::string &msg) -> bool {
  boost::system::error_code ec;
  std::unique_lock<std::mutex> lck(mtxSocket_);
  boost::asio::write(socket_, boost::asio::buffer(msg, msg.size()), ec);
  lck.unlock();
  if (ec == boost::asio::error::broken_pipe) {
    return false; // server has been disconnected
  }
  if (ec) {
    throw boost::system::system_error(ec); // Some other error.
  }
  return true;
}

auto Client::CanWeWriteToServer() -> void {
  const auto msg = msg::GetIsAliveMsg();
  boost::system::error_code ec;
  for (;;) {
    std::unique_lock<std::mutex> lck(mtxSocket_);
    boost::asio::write(socket_, boost::asio::buffer(msg, msg.size()), ec);
    lck.unlock();
    if (ec) {
      return;
    }
    LongSleep();
  }
}

auto Client::ReadMessageSize() -> std::optional<int32_t> {
  // Read in msg header size that can have up to 4 characters representing
  // digits 0/9
  const auto msgSize = DoRead(msg::nrDigitsInMsgHeader);
  return (msgSize != std::nullopt) ? std::make_optional(std::stoi(*msgSize))
                                   : std::nullopt;
}

auto Client::ReadMessageBody(int32_t msgSize) -> std::optional<std::string> {
  // Read in msg body
  return DoRead(msgSize);
}

auto Client::Exit() -> void {
  boost::system::error_code ec;
  std::lock_guard<std::mutex> lck(mtxSocket_);
  socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                   ec); // ignore errors
  socket_.close(ec);
}

auto Client::Read() -> void {
  for (;;) {
    const auto sizeMsg = ReadMessageSize();
    if (sizeMsg == std::nullopt) {
      break;
    }
    const auto msg = ReadMessageBody(*sizeMsg);
    if (msg == std::nullopt) {
      break;
    }
    PrintMessage(*msg);
  }
}

auto Client::Write() -> void {
  AsyncGetline ag;
  for (;;) {
    if (IsReady(isDeadFut_)) {
      break;
    }
    const auto message = ag.GetLine();
    if (!message.empty()) {
      if (exitCommand == ToLower(message)) {
        Exit();
        break;
      }
      const auto composedMsg = msg::EncodeHeader(message);
      const auto write = DoWrite(composedMsg);
      if (!write) {
        break;
      }
    }
    QuickSleep();
  }
}
// Run launches separate threads to read/write and returns after runFor
// duration
auto Client::Run(const HostPort &hostport) -> void {
  boost::asio::ip::tcp::resolver resolver(ioContext_);
  auto endpoints = resolver.resolve(hostport.host, hostport.port);
  boost::asio::connect(socket_, endpoints);
  PrintWelcomeMessage();
  isDeadFut_ =
      std::async(std::launch::async, &Client::CanWeWriteToServer, this);
  readFut_ = std::async(std::launch::async, &Client::Read, this);
  writeFut_ = std::async(std::launch::async, &Client::Write, this);

  isDeadFut_.wait();
  readFut_.wait();
  writeFut_.wait();
}

Client::~Client() { Exit(); }

} // namespace client