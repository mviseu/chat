#pragma once
#include "ClientWorker.h"
#include <atomic>
#include <boost/asio.hpp>
#include <memory>
#include <vector>

namespace server {

class Server {
public:
  Server(int maxNrClients);
  auto Run(int port) -> void;
  ~Server();

private:
  auto Server::RunWorkThread() -> void;
  int maxNrClients_ = 0;
  boost::asio::io_context ioContext_;
  std::shared_pointer<boost::asio::io_context::work> work_;
  std::atomic<int> nrClients_;
  std::vector<std::shared_ptr<ClientWorker>> clients_;
  std::vector<std::future<void>> runners_;
};

} // namespace server