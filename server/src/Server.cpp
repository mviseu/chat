#include "Server.h"
#include <algorithm>
#include <cassert>
#include <iostream>

namespace server {

using namespace boost::asio::ip;

auto Server::RunWorkThread() -> void {
  try {
    ioContext_.run();
  } catch (...) {
    throw;
  }
}

Server::Server(int maxNrClients)
    : maxNrClients_(maxNrClients),
      work_(std::make_shared<boost::asio::io_context::work>(ioContext_)),
      nrClients_(0) {
  assert(maxNrClients >= 1);
}

Server::~Server() {
  work_.reset();
  ioContext_.stop();
  std::for_each(runners_.begin(), runners_.end(),
                [](const auto &runner) { runner.get(); });
  // get all exceptions;
}

auto Server::Run(int port) -> void {
  // generate all the worker threads
  std::generate_n(std::back_inserter(runners), maxNrClients,
                  std::async(std::launch::async, &RunWorkThread, this));

  // try doconnect
  tcp::endpoint endpoint(tcp::v4(), port);
  tcp::acceptor acceptor(ioContext_, endpoint);
  std::cout << "Waiting for clients..." << std::endl;
  for (;;) {
    tcp::socket socket(ioContext_);
    acceptor.accept(socket);
    std::cout << "New client joined! ";
    std::lock_guard<std::mutex> lock(mtxClients_);
    clients_.emplace_back(std::move(socket));
    std::cout << clients_.size() << " total clients" << std::endl;
  }
}

} // namespace server