/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 *  @date 29.03.2021
 */

#pragma once

#include <atomic>

#include "detail/byte_container.hpp"
#include "net/multiplexer.hpp"
#include "net/operation.hpp"
#include "net/socket.hpp"
#include "net/socket_manager.hpp"

namespace net {

class socket_manager_impl : public socket_manager {
public:
  // -- constructors -----------------------------------------------------------

  socket_manager_impl(socket handle, multiplexer* mpx, bool mirror,
                      std::atomic<size_t>& received_bytes,
                      std::atomic<size_t>& sent_bytes,
                      std::atomic<size_t>& num_handled_events);

  ~socket_manager_impl();

  // -- event handling ---------------------------------------------------------

  bool handle_read_event() override;

  bool handle_write_event() override;

private:
  bool mirror_;
  detail::byte_buffer write_buffer_;

  // local store of the performance metrics
  size_t received_bytes_;
  size_t sent_bytes_;
  size_t num_handled_events_;

  // Store the global performance metrics of the benchmark
  std::atomic<size_t>& glob_received_bytes_;
  std::atomic<size_t>& glob_sent_bytes_;
  std::atomic<size_t>& glob_num_handled_events_;
};

} // namespace net
