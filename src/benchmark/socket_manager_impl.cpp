/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 *  @date 30.03.2021
 */

#include "benchmark/socket_manager_impl.hpp"

#include <iostream>
#include <memory>

#include "net/tcp_stream_socket.hpp"

namespace benchmark {

using namespace net;

socket_manager_impl::socket_manager_impl(net::socket handle, multiplexer* mpx,
                                         bool mirror, result_ptr results)
  : socket_manager(handle, mpx), mirror_(mirror), results_(std::move(results)) {
  // nop
}

socket_manager_impl::~socket_manager_impl() {
  // nop
}

bool socket_manager_impl::handle_read_event() {
  for (int i = 0; i < 20; ++i) {
    auto read_res = read(handle<tcp_stream_socket>(), receive_buf_);
    if (read_res == 0)
      return false;
    if (read_res < 0) {
      if (last_socket_error_is_temporary()) {
        return true;
      } else {
        mpx()->handle_error(detail::error(detail::socket_operation_failed,
                                          "socket_manager.read(): "
                                            + last_socket_error_as_string()));
        return false;
      }
    }
    results_->add_received_bytes(read_res);
    if (mirror_) {
      write_buf_.insert(write_buf_.end(), receive_buf_.begin(),
                        receive_buf_.end());
      register_writing();
    }
  }
  return true;
}

bool socket_manager_impl::handle_write_event() {
  for (int i = 0; i < 20; ++i) {
    auto write_res = write(handle<tcp_stream_socket>(), write_buf_);
    if (write_res > 0) {
      write_buf_.erase(write_buf_.begin(), write_buf_.begin() + write_res);
      results_->add_sent_bytes(write_res);
      return write_buf_.empty();
    } else if (write_res <= 0) {
      if (last_socket_error_is_temporary())
        return true;
      else
        mpx()->handle_error(detail::error(detail::socket_operation_failed,
                                          "socket_manager.write(): "
                                            + last_socket_error_as_string()));
    }
  }
  return false;
}

} // namespace benchmark
