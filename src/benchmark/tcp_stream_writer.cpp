/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 *  @date 07.04.2021
 */

#include "benchmark/tcp_stream_writer.hpp"

#include <iostream>
#include <variant>

#include "detail/socket_guard.hpp"
#include "net/tcp_stream_socket.hpp"

namespace benchmark {

using detail::none;

tcp_stream_writer::tcp_stream_writer(std::atomic<size_t>& received_bytes,
                                     std::atomic<size_t>& sent_bytes)
  : running_(false),
    received_bytes_(0),
    sent_bytes_(0),
    glob_received_bytes_(received_bytes),
    glob_sent_bytes_(sent_bytes) {
  // nop
}

tcp_stream_writer::~tcp_stream_writer() {
  glob_sent_bytes_ += sent_bytes_;
  glob_received_bytes_ += received_bytes_;
  close(handle_);
}

detail::error tcp_stream_writer::init(const std::string ip,
                                      const uint16_t port) {
  auto connection_res = net::make_connected_tcp_stream_socket(ip, port);
  if (auto err = std::get_if<detail::error>(&connection_res))
    return *err;
  handle_ = std::get<net::tcp_stream_socket>(connection_res);
  return none;
}

void tcp_stream_writer::start() {
  running_ = true;
  reader_thread_ = std::thread(&tcp_stream_writer::read, this);
  writer_thread_ = std::thread(&tcp_stream_writer::write, this);
}

void tcp_stream_writer::stop() {
  running_ = false;
}

void tcp_stream_writer::join() {
  if (writer_thread_.joinable())
    writer_thread_.join();
  if (reader_thread_.joinable())
    reader_thread_.join();
}

void tcp_stream_writer::read() {
  detail::byte_array<8192> read_buffer_;
  while (running_) {
    auto read_res = net::read(handle_, read_buffer_);
    if (read_res <= 0) {
      if (!net::last_socket_error_is_temporary()) {
        std::cerr << "ERROR read failed: " << net::last_socket_error_as_string()
                  << std::endl;
        running_ = false;
      }
    }
    received_bytes_ += read_res;
  }
}

void tcp_stream_writer::write() {
  detail::byte_array<8192> write_buffer_;
  while (running_) {
    auto written = net::write(handle_, write_buffer_);
    if (written <= 0) {
      if (!net::last_socket_error_is_temporary()) {
        std::cerr << "ERROR write failed: "
                  << net::last_socket_error_as_string() << std::endl;
        running_ = false;
      }
    }
    sent_bytes_ += written;
  }
}

} // namespace benchmark
