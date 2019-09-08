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
  acceptor_.bind(endpoint);
  acceptor_.listen();
  assert(maxNrClients_ >= 1);
}

Server::~Server() {
  acceptor_.cancel();
  acceptor_.close();
}

auto Server::DoWriteHandler(const boost::system::error_code &ec) -> void {
  // TODO read transport not connected in doc
  if (ec.value() == 32) {
    return;
  }
  if (ec) {
    throw boost::system::system_error(ec); // Some other error.
  }
}

auto Server::WriteMessage(int clientIndex, const std::string &msg) -> void {
  if (clients_[clientIndex]->socket.is_open()) {
    const auto msgWithHeader = msg::EncodeHeader(msg);
    boost::asio::async_write(
        clients_[clientIndex]->socket,
        boost::asio::buffer(msgWithHeader, msgWithHeader.size()),
        clients_[clientIndex]->strand.wrap(
            [this](const auto &ec, auto) { DoWriteHandler(ec); }));
  }
}

auto Server::WriteToAll(const std::string &msg) -> void {
  for (size_t i = 0; i < clients_.size(); ++i) {
    clients_[i]->strand.post([this, i, msg]() { WriteMessage(i, msg); });
  }
}

auto Server::DoMessageBodyHandler(int clientIndex,
                                  const boost::system::error_code &ec,
                                  const std::string &msg) -> void {
  if (ec == boost::asio::error::eof) {
    return; // Connection closed cleanly by peer.
  }
  if (ec) {
    throw boost::system::system_error(ec); // Some other error.
  }
  WriteToAll(msg);
  clients_[clientIndex]->strand.post(
      [this, clientIndex]() { ReadMessageSize(clientIndex); });
}

auto Server::ReadMessageBody(int clientIndex, int msgSize) -> void {
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
  if (ec == boost::asio::error::eof) {
    return; // Connection closed cleanly by peer.
  }
  if (ec) {
    throw boost::system::system_error(ec); // Some other error.
  }

  const auto msgSizeInt = std::stoi(msgSize);

  if (msg::IsThisAPingMessage(msgSizeInt)) {
    clients_[clientIndex]->strand.post([this, clientIndex]() {
      ReadMessageSize(clientIndex);
    }); // go back to start
  } else {
    clients_[clientIndex]->strand.post([this, clientIndex, msgSizeInt]() {
      ReadMessageBody(clientIndex, msgSizeInt);
    });
  }
}

auto Server::ReadMessageSize(int clientIndex) -> void {
  auto buf(std::make_shared<std::string>(msg::nrDigitsInMsgHeader, '\0'));
  boost::asio::async_read(clients_[clientIndex]->socket,
                          boost::asio::buffer(*buf, msg::nrDigitsInMsgHeader),
                          clients_[clientIndex]->strand.wrap(
                              [this, clientIndex, buf](const auto &ec, auto) {
                                DoMessageSizeHandler(clientIndex, ec, *buf);
                              }));
}

auto Server::DoAccept(int clientIndex) -> void {
  acceptor_.async_accept(
      [this, clientIndex](boost::system::error_code ec,
                          boost::asio::ip::tcp::socket socket) {
        if (ec == boost::asio::error::operation_aborted) {
          // if acceptor has been canceled stop here
          return;
        }
        if (!ec) {
          clients_[clientIndex]->socket = std::move(socket);
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
  // generate all the clients
  std::generate_n(std::back_inserter(clients_), maxNrClients_, [this]() {
    return std::make_shared<ClientWorker>(ioContext_);
  });

  // generate all the worker threads
  std::generate_n(std::back_inserter(runners_), maxNrClients_, [this]() {
    return std::async(std::launch::async, &Server::RunWorkThread, this);
  });

  DoAccept(0);

  std::for_each(runners_.begin(), runners_.end(),
                [](auto &runner) { runner.wait(); });
}

} // namespace server