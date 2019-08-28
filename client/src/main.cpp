#include "Connect.h"
#include "HostPort.h"
#include <boost/asio.hpp>
#include <iostream>

int main() {
  try {
    client::HostPort hostport("localhost", "3000");
    boost::asio::io_context ioContext;
    boost::asio::ip::tcp::socket socket(ioContext);
    client::Connect(hostport, ioContext, socket);

    // then start the multithreading here by having some thread to read and
    // another to write
    return 0;
  } catch (...) {
    throw;
  }
}