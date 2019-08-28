#include "AcceptConnections.h"
#include <iostream>

namespace server {

auto AcceptConnections(int port, boost::asio::io_context &ioContext,
                       std::vector<boost::asio::ip::tcp::socket> &clients,
                       std::mutex &mtx) -> void {
  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
  boost::asio::ip::tcp::acceptor acceptor(ioContext, endpoint);
  std::cout << "Waiting for clients..." << std::endl;
  for (;;) {
    boost::asio::ip::tcp::socket socket(ioContext);
    acceptor.accept(socket);
    std::cout << "New client joined! ";
    std::lock_guard<std::mutex> lock(mtx);
    clients.emplace_back(std::move(socket));
    std::cout << clients.size() << " total clients" << std::endl;
  }
}

} // namespace server