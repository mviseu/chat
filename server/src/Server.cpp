#include "Server.h"
#include <iostream>

namespace server {

using namespace boost::asio::ip;

auto Server::Listen(int port) -> void {
  tcp::endpoint endpoint(tcp::v4(), port);
  tcp::acceptor acceptor(ioContext_, endpoint);
  std::cout << "Waiting for clients..." << std::endl;
  for (;;) {
    tcp::socket socket(ioContext_);
    acceptor.accept(socket);
    std::cout << "New client joined! ";
    std::lock_guard<std::mutex> lock(mtxClients_);
    clients_.emplace_back(std::move(socket));
    std::cout << clients_.size() << " total clients" << std::endl;
  }
}

} // namespace server
