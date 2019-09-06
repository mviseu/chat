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
  auto MoveSocket(int clientIndex, boost::asio::ip::tcp::socket socket) -> void;
  auto PostSocket(int clientIndex, boost::asio::ip::tcp::socket socket) -> void;
  auto PostAccept() -> void;
  auto DoAccept() -> void;
  auto RunWorkThread() -> void;

  int maxNrClients_ = 0;
  boost::asio::io_context ioContext_;
  std::shared_ptr<boost::asio::io_context::work> work_;
  boost::asio::ip::tcp::acceptor acceptor_;
  std::atomic<int> nrClients_;
  std::vector<std::shared_ptr<ClientWorker>> clients_;
  std::vector<std::future<void>> runners_;
};

} // namespace server