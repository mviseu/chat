#include "AcceptConnections.h"

namespace server {
auto AcceptConnections(int port, boost::asio::io_context &ioContext,
                       Sockets sockets) -> void {
  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
  boost::asio::ip::tcp::acceptor acceptor(ioContext, endpoint);
}
} // namespace server