#include "Server.h"
#include "Message.h"
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
    std::cout << "after stop before throw" << std::endl;
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
}

auto Server::DoWriteHandler(int clientIndex,
                            const boost::system::error_code &ec) -> void {
  std::cout << "Start do write handler for client " << clientIndex << std::endl;
  if (ec) {
    throw boost::system::system_error(ec); // Some other error.
  }
  std::cout << "End do write handler for client " << clientIndex << std::endl;
}

auto Server::WriteMessage(int clientIndex, const std::string &msg) -> void {
  std::cout << "Start write for client" << clientIndex << std::endl;
  const auto msgWithHeader = msg::EncodeHeader(msg);
  boost::asio::async_write(
      clients_[clientIndex]->socket,
      boost::asio::buffer(msgWithHeader, msgWithHeader.size()),
      clients_[clientIndex]->strand.wrap(
          [this, clientIndex](const auto &ec, auto) {
            DoWriteHandler(clientIndex, ec);
          }));
  std::cout << "End write for client " << clientIndex << std::endl;
}

auto Server::WriteToAll(const std::string &msg) -> void {
  for (size_t i = 0; i < clients_.size(); ++i) {
    clients_[i]->strand.post([this, i, msg]() { WriteMessage(i, msg); });
  }
}

auto Server::DoMessageBodyHandler(int clientIndex,
                                  const boost::system::error_code &ec,
                                  const std::string &msg) -> void {
  std::cout << "Start Message body handler for client " << clientIndex
            << std::endl;
  if (ec == boost::asio::error::eof) {
    std::cout << "Connection closed for client " << clientIndex << std::endl;
    return; // Connection closed cleanly by peer.
  }
  if (ec) {
    throw boost::system::system_error(ec); // Some other error.
  }
  std::cout << "Message for client " << clientIndex << " is " << msg
            << std::endl;

  WriteToAll(msg);
  clients_[clientIndex]->strand.post(
      [this, clientIndex]() { ReadMessageSize(clientIndex); });
}

auto Server::ReadMessageBody(int clientIndex, int msgSize) -> void {
  std::cout << "Start read message body for client" << clientIndex << std::endl;
  msg::CheckHeaderSize(msgSize);
  auto buf(std::make_shared<std::string>(msgSize, '\0'));
  boost::asio::async_read(clients_[clientIndex]->socket,
                          boost::asio::buffer(*buf, msgSize),
                          clients_[clientIndex]->strand.wrap(
                              [this, clientIndex, buf](const auto &ec, auto) {
                                DoMessageBodyHandler(clientIndex, ec, *buf);
                              }));
}

auto Server::DoMessageSizeHandler(int clientIndex,
                                  const boost::system::error_code &ec,
                                  const std::string &msgSize) -> void {
  std::cout << "Start Message size handler for client " << clientIndex
            << std::endl;
  if (ec == boost::asio::error::eof) {
    std::cout << "Connection closed for client " << clientIndex << std::endl;
    return; // Connection closed cleanly by peer.
  }
  if (ec) {
    std::cout << "Do message size handler for client index " << clientIndex
              << " with error ";
    std::cout << "socket is " << clients_[clientIndex]->socket.is_open()
              << std::endl;
    throw boost::system::system_error(ec); // Some other error.
  }
  std::cout << "no error code in message size for client " << clientIndex
            << std::endl;
  const auto msgSizeInt = std::stoi(msgSize);
  std::cout << "message size is " << msgSizeInt << std::endl;
  clients_[clientIndex]->strand.post([this, clientIndex, msgSizeInt]() {
    ReadMessageBody(clientIndex, msgSizeInt);
  });
}

auto Server::ReadMessageSize(int clientIndex) -> void {
  std::cout << "Start read message size for client" << clientIndex << std::endl;
  auto buf(std::make_shared<std::string>(msg::nrDigitsInMsgHeader, '\0'));
  boost::asio::async_read(
      clients_[clientIndex]->socket,
      boost::asio::buffer(*buf, msg::nrDigitsInMsgHeader),
      clients_[clientIndex]->strand.wrap(
          [this, clientIndex, buf](const auto &ec, auto nrBytes) {
            std::cout << "nr bytes read is " << nrBytes << std::endl;
            std::cout << "message size is " << *buf << std::endl;
            DoMessageSizeHandler(clientIndex, ec, *buf);
          }));
  std::cout << "End ReadMessageSize for client " << clientIndex << std::endl;
}

auto Server::DoAccept(int clientIndex) -> void {
  std::cout << "Start DoAccept on strand" << clientIndex << std::endl;
  acceptor_.async_accept(
      [this, clientIndex](boost::system::error_code ec,
                          boost::asio::ip::tcp::socket socket) {
        std::cout << "Do Accept handler" << std::endl;
        if (ec == boost::asio::error::operation_aborted) {
          // if acceptor has been canceled stop here
          return;
        }
        if (!ec) {
          std::cout << "Accept successful: start reading from a new client "
                    << clientIndex << std::endl;
          clients_[clientIndex]->socket = std::move(socket);
          std::cout << "is socket open "
                    << clients_[clientIndex]->socket.is_open() << std::endl;

          std::cout << "post for client " << clientIndex << std::endl;
          clients_[clientIndex]->strand.post(
              [this, clientIndex]() { ReadMessageSize(clientIndex); });

          ++nrClients_;
          if (static_cast<size_t>(nrClients_) < runners_.size()) {
            DoAccept(clientIndex + 1);
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

  DoAccept(0);
  std::cout << "After post accept" << std::endl;
  std::for_each(runners_.begin(), runners_.end(),
                [](auto &runner) { runner.wait(); });
}

} // namespace server