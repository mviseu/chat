#pragma once
#include <string>

namespace client {
struct HostPort {
  HostPort(std::string host, std::string port);
  std::string host;
  std::string port;
};
} // namespace client
