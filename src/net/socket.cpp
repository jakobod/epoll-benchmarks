/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 *  @date 04.03.2021
 *
 *  This file is based on `socket.cpp` from the C++ Actor Framework.
 *  https://github.com/actor-framework/incubator
 */

#include "net/socket.hpp"

#include <cstring>
#include <iostream>
#include <net/socket_sys_includes.hpp>
#include <sys/ioctl.h>

namespace net {

void close(socket hdl) {
  if (hdl != invalid_socket)
    ::close(hdl.id);
}

void shutdown(socket hdl, int how) {
  if (hdl != invalid_socket)
    ::shutdown(hdl.id, how);
}

int last_socket_error() {
  return errno;
}

bool last_socket_error_is_temporary() {
  auto code = last_socket_error();
#if EAGAIN == EWOULDBLOCK
  return code == EAGAIN;
#else
  return code == EAGAIN || code == EWOULDBLOCK;
#endif
}

std::string last_socket_error_as_string() {
  return strerror(errno);
}

bool nonblocking(socket hdl, bool new_value) {
  // read flags for x
  auto flags = fcntl(hdl.id, F_GETFL, 0);
  if (flags == -1)
    return false;
  flags = new_value ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
  return (fcntl(hdl.id, F_SETFL, flags) == 0);
}

/// Returns the mac address of the specified interface.
ifreq get_if_mac(socket hdl, const std::string& if_name) {
  ifreq if_mac = {};
  strncpy(if_mac.ifr_name, if_name.c_str(), IFNAMSIZ - 1);
  if (ioctl(hdl.id, SIOCGIFHWADDR, &if_mac) < 0)
    perror("SIOCGIFHWADDR");
  return if_mac;
}

/// Returns the index of the specified interface.
ifreq get_if_index(socket hdl, const std::string& if_name) {
  ifreq if_index = {};
  strncpy(if_index.ifr_name, if_name.c_str(), IFNAMSIZ - 1);
  if (ioctl(hdl.id, SIOCGIFINDEX, &if_index) < 0)
    perror("SIOCGIFINDEX");
  return if_index;
}

} // namespace net
