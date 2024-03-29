#pragma once
#include "ClientWorker.h"
#include <atomic>
#include <boost/asio.hpp>
#include <memory>
#include <vector>

namespace server {

class Server {
public:
  Server(int maxNrClients, int port);
  auto Run() -> void;
  ~Server();

private:
  auto DoWriteHandler(const boost::system::error_code &ec) -> void;
  auto WriteMessage(int clientIndex, const std::string &msg) -> void;
  auto WriteToAll(int clientIndex, const std::string &msg) -> void;
  auto DoMessageBodyHandler(int clientIndex,
                            const boost::system::error_code &ec,
                            const std::string &msg) -> void;

  auto ReadMessageBody(int clientIndex, int msgSize) -> void;
  auto DoMessageSizeHandler(int clientIndex,
                            const boost::system::error_code &ec,
                            const std::string &msgSize) -> void;

  auto ReadMessageSize(int clientIndex) -> void;
  auto DoAccept(int ClientIndex) -> void;
  auto RunWorkThread() -> void;
  auto Read(int clientIndex) -> void;

  int maxNrClients_ = 0;
  boost::asio::io_context ioContext_;
  std::shared_ptr<boost::asio::io_context::work> work_;
  boost::asio::ip::tcp::acceptor acceptor_;
  std::atomic<int> nrClients_;
  std::vector<std::shared_ptr<ClientWorker>> clients_;
  std::vector<std::future<void>> runners_;
};

} // namespace server