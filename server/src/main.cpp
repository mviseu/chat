#include "Server.h"
#include <mutex>
#include <thread>

int main() {
  try {
    server::Server server(10, 3000);
    server.Run();
    return 0;
  } catch (...) {
    throw;
  }
}