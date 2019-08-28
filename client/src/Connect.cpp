#include "Connect.h"

namespace client {

// creates a socket to connect using host and port
// if unsuccessful must stop
// returns either a successful socket or a failure
auto Connect(const HostPort &hostport, boost::asio::io_context &ioContext,
             boost::asio::ip::tcp::socket &socket) -> void {
  // can we do with const here?
  boost::asio::ip::tcp::resolver resolver(ioContext);
  auto endpoints = resolver.resolve(hostport.host, hostport.port);
  boost::asio::connect(socket, endpoints);
}

} // namespace client