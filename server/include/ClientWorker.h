#pragma once

#include <boost/asio.hpp>
#include <future>
#include <memory>

namespace server {

class ClientWorker {
public:
  ClientWorker(boost::asio::io_context &ioContext);
  ~ClientWorker();

private:
  // have a strand per socket to ensure sequential ops and safety accessing the
  // socket
  boost::asio::io_context::strand strand_;
  // one socket per client
  boost::asio::ip::tcp::socket socket_;
};

} // namespace server