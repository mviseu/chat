#include "Client.h"
#include "HostPort.h"
#include <boost/asio.hpp>
#include <iostream>

int main() {
  try {
    client::HostPort hostport("localhost", "3000");
    client::Client client;
    client.Run(hostport, std::chrono::seconds(10));
    return 0;
  } catch (...) {
    throw;
  }
}