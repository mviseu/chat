#include "Sockets.h"

namespace server {
Sockets::Sockets(std::vector<boost::asio::ip::tcp::socket> socketss)
    : sockets(std::move(socketss)) {}
} // namespace server