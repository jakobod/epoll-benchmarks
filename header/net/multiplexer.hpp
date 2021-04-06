/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 *  @date 30.03.2021
 */

#pragma once

#include <array>
#include <sys/epoll.h>
#include <thread>
#include <unordered_map>

#include "detail/error.hpp"
#include "net/fwd.hpp"
#include "net/operation.hpp"

namespace net {

static const size_t max_epoll_events = 32;

class multiplexer {
  using pollset = std::array<epoll_event, max_epoll_events>;
  using epoll_fd = int;
  using manager_map = std::unordered_map<int, socket_manager_ptr>;

public:
  // -- constructors, destructors ----------------------------------------------

  multiplexer();

  ~multiplexer();

  /// Initializes the multiplexer.
  detail::error init();

  /// Creates a thread that runs this multiplexer indefinately.
  void start();

  // -- Interface functions ----------------------------------------------------

  /// Adds a new fd to the multiplexer for operation `initial`.
  void add(socket_manager_ptr mgr, operation initial);

  void enable(socket_manager&, operation op);

  void disable(socket_manager& mgr, operation op);

private:
  /// Deletes an existing fd.
  void del(socket handle);

  /// Modifies the epollset for existing fds.
  void mod(int fd, int op, operation events);

  void run();

  // epoll variables
  epoll_fd epoll_fd_;
  pollset pollset_;
  manager_map managers_;

  // thread variables
  bool running_;
  std::thread mpx_thread_;
  std::thread::id mpx_thread_id_;
};

} // namespace net
