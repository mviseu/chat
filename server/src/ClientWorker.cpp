#include "ClientWorker.h"

namespace server {

ClientWorker::ClientWorker(boost::asio::io_context &ioContext)
    : strand(ioContext), socket(ioContext) {}

ClientWorker::~ClientWorker() {
  boost::system::error_code ec;
  socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                  ec); // ignore errors
  socket.close(ec);    // ignore errors
}
} // namespace server