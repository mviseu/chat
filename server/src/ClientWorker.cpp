#include "ClientWorker.h"

namespace server {

ClientWorker::ClientWorker(boost::asio::io_context &ioContext)
    : strand_(ioContext), socket_(ioContext) {}

ClientWorker::~ClientWorker() {
  socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
  socket_.close();
} // namespace server