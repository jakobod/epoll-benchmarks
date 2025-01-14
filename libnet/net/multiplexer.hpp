/**
 *  @author    Jakob Otto
 *  @file      multiplexer.hpp
 *  @copyright Copyright 2023 Jakob Otto. All rights reserved.
 *             This file is part of the network-driver project, released under
 *             the GNU GPL3 License.
 */

#pragma once

#include "net/fwd.hpp"

#include "net/ip/v4_endpoint.hpp"

#include "net/operation.hpp"
#include "net/socket/tcp_stream_socket.hpp"
#include "net/socket_manager.hpp"

#include "util/config.hpp"
#include "util/error.hpp"
#include "util/error_or.hpp"
#include "util/intrusive_ptr.hpp"

#include <chrono>
#include <cstdint>

namespace net {

class multiplexer {
public:
  virtual ~multiplexer() = default;

  /// Initializes the multiplexer.
  virtual util::error
  init(socket_manager_factory_ptr factory, const util::config& cfg)
    = 0;

  /// Creates a thread that runs this multiplexer indefinately.
  virtual void start() = 0;

  /// Shuts the multiplexer down!
  virtual void shutdown() = 0;

  /// Joins with the multiplexer.
  virtual void join() = 0;

  virtual bool running() const = 0;

  // -- Error Handling ---------------------------------------------------------

  /// Handles an error `err`.
  virtual void handle_error(const util::error& err) = 0;

  // -- Interface functions ----------------------------------------------------

  /// The main multiplexing loop.
  virtual util::error poll_once(bool blocking) = 0;

  /// Adds a new fd to the multiplexer for operation `initial`.
  /// @warning This function is *NOT* thread-safe.
  virtual void add(socket_manager_ptr mgr, operation initial) = 0;

  /// Enables an operation `op` for socket manager `mgr`.
  /// @warning This function is *NOT* thread-safe.
  virtual void enable(socket_manager_ptr mgr, operation op) = 0;

  /// Disables an operation `op` for socket manager `mgr`.
  /// If `mgr` is not registered for any operation after disabling it, it is
  /// removed if `remove` is set.
  /// @warning This function is *NOT* thread-safe.
  virtual void disable(socket_manager_ptr mgr, operation op, bool remove) = 0;

  /// Sets a timeout for socket_manager `mgr` at timepoint `when` and returns
  /// the id.
  /// @warning This function is *NOT* thread-safe.
  virtual uint64_t set_timeout(socket_manager_ptr mgr,
                               std::chrono::system_clock::time_point when)
    = 0;

  template <class Manager, class... Ts>
  util::error
  tcp_connect(const ip::v4_endpoint& ep, operation initial_op, Ts&&... xs) {
    auto sock = make_connected_tcp_stream_socket(ep);
    if (auto err = util::get_error(sock))
      return *err;
    auto mgr = util::make_intrusive<Manager>(std::get<tcp_stream_socket>(sock),
                                             this, std::forward<Ts>(xs)...);
    add(mgr, initial_op);
    return util::none;
  }

  // -- members ----------------------------------------------------------------

  /// Returns the port the multiplexer is listening on.
  constexpr std::uint16_t port() const noexcept { return port_; }

protected:
  std::uint16_t port_{0};
};

} // namespace net
