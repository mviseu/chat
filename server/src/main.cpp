#include "AcceptConnections.h"
#include "Sockets.h"
#include <mutex>
#include <thread>

int main() {
  try {
    // create a thread to accept incoming connections
    boost::asio::io_context ioContext;
    Sockets sockets({});
    server::AcceptConnections(1009, ioContext, sockets);
    ioContext.run(); // run accept connections in the main thread
    return 0;
  } catch (...) {
    throw;
  }
}