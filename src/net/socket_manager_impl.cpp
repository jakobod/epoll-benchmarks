/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 *  @date 30.03.2021
 */

#include "net/socket_manager_impl.hpp"

#include <iostream>
#include <memory>

#include "detail/byte_container.hpp"
#include "net/tcp_stream_socket.hpp"

namespace net {

socket_manager_impl::socket_manager_impl(
  socket handle, multiplexer* mpx, bool mirror,
  std::atomic<size_t>& received_bytes, std::atomic<size_t>& sent_bytes,
  std::atomic<size_t>& num_handled_events)
  : socket_manager(handle, mpx),
    mirror_(mirror),
    received_bytes_(0),
    sent_bytes_(0),
    num_handled_events_(0),
    glob_received_bytes_(received_bytes),
    glob_sent_bytes_(sent_bytes),
    glob_num_handled_events_(num_handled_events) {
  // nop
}

socket_manager_impl::~socket_manager_impl() {
  glob_sent_bytes_ += sent_bytes_;
  glob_received_bytes_ += received_bytes_;
  glob_num_handled_events_ += num_handled_events_;
}

bool socket_manager_impl::handle_read_event() {
  static constexpr int max_reads = 20;
  ++num_handled_events_;
  detail::byte_array<2048> buf;
  for (int i = 0; i < max_reads; ++i) {
    auto res = read(handle<tcp_stream_socket>(), buf);
    if (res == 0)
      return false;
    if (res < 0) {
      if (last_socket_error_is_temporary()) {
        return true;
      } else {
        mpx()->handle_error(detail::error(detail::socket_operation_failed,
                                          "socket_manager.read(): "
                                            + last_socket_error_as_string()));
        return false;
      }
    }
    received_bytes_ += res;
    if (mirror_) {
      write_buffer_.insert(write_buffer_.begin(), buf.begin(),
                           buf.begin() + res);
      register_writing();
    }
  }
  return true;
}

bool socket_manager_impl::handle_write_event() {
  ++num_handled_events_;
  auto written = write(handle<tcp_stream_socket>(), write_buffer_);
  if (written > 0) {
    write_buffer_.erase(write_buffer_.begin(), write_buffer_.begin() + written);
    sent_bytes_ += written;
    return !write_buffer_.empty();
  } else if (written < 0) {
    if (last_socket_error_is_temporary())
      return true;
    else if (last_socket_error() == EPIPE)
      return false;
    else
      mpx()->handle_error(detail::error(detail::socket_operation_failed,
                                        "socket_manager.write(): "
                                          + last_socket_error_as_string()));
  }
  return false;
}

} // namespace net
