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
  std::cout << "after bind" << std::endl;
  assert(maxNrClients_ >= 1);
}

Server::~Server() {
  work_.reset();
  acceptor_.cancel();
  acceptor_.close();
  ioContext_.stop();
  std::for_each(runners_.begin(), runners_.end(),
                [](auto &runner) { runner.get(); });

  // get all exceptions;
}

auto Server::MoveSocket(int clientIndex, boost::asio::ip::tcp::socket socket)
    -> void {
  clients_[clientIndex]->socket = std::move(socket);
}

auto Server::PostSocket(int clientIndex, boost::asio::ip::tcp::socket socket)
    -> void {
  clients_[clientIndex]->strand.post([this, clientIndex, &socket]() {
    this->MoveSocket(clientIndex, std::move(socket));
  });
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
      PostSocket(nrClients_, std::move(socket));
      ++nrClients_;
    }
    std::cout << ec.value() << std::endl;
    PostAccept();
  });
}

auto Server::PostAccept() -> void {
  std::cout << "Post Accept" << std::endl;
  clients_[nrClients_]->strand.post(boost::bind(&Server::DoAccept, this));
  std::cout << "end post accept" << std::endl;
}

auto Server::Run() -> void {
  // generate all the worker threads
  std::generate_n(std::back_inserter(runners_), maxNrClients_, [this]() {
    return std::async(std::launch::async, &Server::RunWorkThread, this);
  });

  // and all the clients
  std::generate_n(std::back_inserter(clients_), maxNrClients_, [this]() {
    return std::make_shared<ClientWorker>(ioContext_);
  });

  std::cout << "Waiting for clients..." << std::endl;

  PostAccept();
  std::cout << "After post accept" << std::endl;
  // wait on all futures? what will be the stop condition?
  std::for_each(runners_.begin(), runners_.end(),
                [](auto &runner) { runner.wait(); });
}

} // namespace server