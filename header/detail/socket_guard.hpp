/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 *  @date 18.02.2021
 */

#pragma once

#include <iostream>

#include "net/socket.hpp"

static constexpr int invalid_fd = -1;

namespace detail {

template <class Socket>
class socket_guard {
public:
  socket_guard(Socket sock) : sock_(sock) {
    // nop
  }

  ~socket_guard() {
    if (sock_ != net::invalid_socket) {
      net::close(sock_);
    }
  }

  Socket release() {
    auto sock = sock_;
    sock_.id = invalid_fd;
    return sock;
  }

  Socket operator*() {
    return sock_;
  }

  template <class Other>
  bool operator==(const Other& other) {
    return sock_ == other;
  }

private:
  Socket sock_;
};

template <class Socket>
socket_guard<Socket> make_socket_guard(Socket sock) {
  return {sock};
}

} // namespace detail
