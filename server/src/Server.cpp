#include "Server.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <cassert>
#include <iostream>

namespace server {

using namespace boost::asio::ip;

auto Server::RunWorkThread() -> void {
  try {
    ioContext_.run();
  } catch (...) {
    std::cout << "Exception" << std::endl;
    work_.reset();
    ioContext_.stop();
    throw;
  }
}

Server::Server(int maxNrClients, int port)
    : maxNrClients_(maxNrClients),
      work_(std::make_shared<boost::asio::io_context::work>(ioContext_)),
      acceptor_(ioContext_), nrClients_(0) {
  boost::asio::ip::tcp::endpoint endpoint(tcp::v4(), port);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  std::cout << "before bind" << std::endl;
  acceptor_.bind(endpoint);
  acceptor_.listen();
  std::cout << "after bind" << std::endl;
  assert(maxNrClients_ >= 1);
}

Server::~Server() {
  std::cout << "start destruction" << std::endl;
  acceptor_.cancel();
  acceptor_.close();

  // get all exceptions;
}

auto Server::Read(int clientIndex) -> void {
  std::cout << "start read from client " << clientIndex << std::endl;
  // boost::asio::async_read();
}

auto Server::DoAccept() -> void {
  std::cout << "Start DoAccept on strand" << nrClients_ << std::endl;
  acceptor_.async_accept([this](boost::system::error_code ec,
                                boost::asio::ip::tcp::socket socket) {
    std::cout << "Do Accept handler" << std::endl;
    if (ec == boost::asio::error::operation_aborted) {
      // if acceptor has been canceled stop here
      return;
    }
    if (!ec) {
      std::cout << "Accept successful: start reading from a new client"
                << std::endl;
      clients_[nrClients_]->socket = std::move(socket);
      clients_[nrClients_]->strand.post([this]() { this->Read(nrClients_); });

      ++nrClients_;
      if (static_cast<size_t>(nrClients_) < runners_.size()) {
        DoAccept();
      }
    } else {
      throw ec;
    }
  });
}

auto Server::Run() -> void {
  // and all the clients
  std::generate_n(std::back_inserter(clients_), maxNrClients_, [this]() {
    return std::make_shared<ClientWorker>(ioContext_);
  });

  // generate all the worker threads
  std::generate_n(std::back_inserter(runners_), maxNrClients_, [this]() {
    return std::async(std::launch::async, &Server::RunWorkThread, this);
  });

  std::cout << "Waiting for clients..." << std::endl;

  DoAccept();
  std::cout << "After post accept" << std::endl;
  // wait on all futures? what will be the stop condition?

  std::for_each(runners_.begin(), runners_.end(),
                [](auto &runner) { runner.wait(); });
}

} // namespace server