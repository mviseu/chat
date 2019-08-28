#include "HostPort.h"
#include <utility>

namespace client {
HostPort::HostPort(std::string hostt, std::string portt)
    : host(std::move(hostt)), port(std::move(portt)) {}
} // namespace client