#include "ClientWorker.h"

namespace server {

ClientWorker::ClientWorker(boost::asio::io_context &ioContext)
    : strand(ioContext), socket(ioContext) {}

ClientWorker::~ClientWorker() {
  socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
  socket.close();
}
} // namespace server