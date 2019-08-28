#include "AcceptConnections.h"
#include <mutex>
#include <thread>

int main() {
  try {
    // create a thread to accept incoming connections
    boost::asio::io_context ioContext;
    std::vector<boost::asio::ip::tcp::socket> clients;
    std::mutex mutClients;
    server::AcceptConnections(3000, ioContext, clients, mutClients);
    return 0;
  } catch (...) {
    throw;
  }
}