/**
 *  @author    Jakob Otto
 *  @file      multiplexer_impl.hpp
 *  @copyright Copyright 2023 Jakob Otto. All rights reserved.
 *             This file is part of the network-driver project, released under
 *             the GNU GPL3 License.
 */

#pragma once

#include "net/fwd.hpp"
#include "util/fwd.hpp"

#include "net/multiplexer.hpp"
#include "net/socket/pipe_socket.hpp"
#include "net/timeout_entry.hpp"

#include "util/binary_serializer.hpp"
#include "util/byte_buffer.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>
#include <set>
#include <span>
#include <thread>
#include <unordered_map>

#if defined(__linux__)
#  define EPOLL_MPX
#  include <sys/epoll.h>
#elif defined(__APPLE__)
#  define KQUEUE_MPX
#  include <sys/event.h>
#endif

namespace net {

/// Implements a multiplexing backend for handling event multiplexing facilities
/// such as epoll and kqueue.
class multiplexer_impl : public multiplexer {
  static constexpr std::size_t max_events = 32;

#if defined(EPOLL_MPX)
  using event_type = epoll_event;
  using mpx_fd = int;
#elif defined(KQUEUE_MPX)
  using event_type = struct kevent;
  using mpx_fd = int;
#endif

  // Pollset types
  using pollset = std::array<event_type, max_events>;
  using update_list = std::vector<event_type>;
  using event_span = std::span<event_type>;
  using manager_map = std::unordered_map<socket_id, socket_manager_ptr>;

  // Timeout handling types
  using optional_timepoint
    = std::optional<std::chrono::system_clock::time_point>;
  using timeout_entry_set = std::set<timeout_entry>;

public:
  // -- constructors, destructors ----------------------------------------------

  multiplexer_impl() = default;

  ~multiplexer_impl() override;

  /// Initializes the multiplexer.
  util::error init(socket_manager_factory_ptr factory,
                   const util::config& cfg) override;

  // -- Thread functions -------------------------------------------------------

  /// Creates a thread that runs this multiplexer indefinately.
  void start() override;

  /// Shuts the multiplexer down!
  void shutdown() override;

  /// Joins with the multiplexer.
  void join() override;

  bool running() const override;

  void set_thread_id(std::thread::id tid = {}) noexcept;

  // -- members ----------------------------------------------------------------

  std::uint16_t num_socket_managers() const { return managers_.size(); }

  // -- Error Handling ---------------------------------------------------------

  void handle_error(const util::error& err) override;

  // -- Interface functions ----------------------------------------------------

  /// Adds a new fd to the multiplexer for operation `initial`.
  void add(socket_manager_ptr mgr, operation initial) override;

  /// Enables an operation `op` for socket manager `mgr`.
  void enable(socket_manager_ptr, operation op) override;

  /// Disables an operation `op` for socket manager `mgr`.
  /// If `mgr` is not registered for any operation after disabling it, it is
  /// removed if `remove` is set.
  void disable(socket_manager_ptr mgr, operation op, bool remove) override;

  std::uint64_t
  set_timeout(socket_manager_ptr mgr,
              std::chrono::system_clock::time_point when) override;

  /// Main multiplexing loop.
  util::error poll_once(bool blocking) override;

private:
  /// Notifies all socket managers about timeouts that have expired.
  void handle_timeouts();

  /// Handles all IO-events that occurred.
  void handle_events(event_span events);

  /// The main multiplexer loop.
  void run();

  /// Deletes an existing socket_manager using its key `handle`.
  void del(socket handle);

  /// Deletes an existing socket_manager using an iterator `it` to the
  /// manager_map.
  manager_map::iterator del(manager_map::iterator it);

  /// Modifies the epollset for existing fds.
  void mod(int fd, int op, operation events);

  /// Writes the pollset_update code to the pipe
  template <class... Ts>
  ptrdiff_t write_to_pipe(Ts&&... ts) {
    util::byte_buffer buf;
    util::binary_serializer bs{buf};
    bs(std::forward<Ts>(ts)...);
    return write(pipe_writer_, util::as_const_bytes(buf));
  }

  bool is_multiplexer_thread() {
    return std::this_thread::get_id() == mpx_thread_id_;
  }

  // pipe for synchronous access to mpx
  pipe_socket pipe_writer_{invalid_socket_id};
  pipe_socket pipe_reader_{invalid_socket_id};

  // Multiplexing variables
  mpx_fd mpx_fd_{invalid_socket_id};
  pollset pollset_;
  update_list update_cache_;
  manager_map managers_;

  // timeout handling
  timeout_entry_set timeouts_;
  optional_timepoint current_timeout_{std::nullopt};
  std::uint64_t current_timeout_id_{0};

  // thread variables
  bool shutting_down_{false};
  bool running_{false};
  std::thread mpx_thread_;
  std::thread::id mpx_thread_id_;

  const util::config* cfg_ = nullptr;
};

util::error_or<multiplexer_ptr>
make_multiplexer(socket_manager_factory_ptr factory, const util::config& cfg);

} // namespace net
