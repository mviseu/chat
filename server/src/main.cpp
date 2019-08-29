#include "Server.h"
#include <mutex>
#include <thread>

int main() {
  try {
    server::Server server;
    server.Listen(3000);
    return 0;
  } catch (...) {
    throw;
  }
}