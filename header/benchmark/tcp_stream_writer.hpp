/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 *  @date 08.04.2021
 */

#pragma once

#include <thread>

#include "detail/error.hpp"
#include "net/tcp_stream_socket.hpp"

namespace benchmark {

class tcp_stream_writer {
public:
  tcp_stream_writer();
  tcp_stream_writer(std::atomic<size_t>& received_bytes,
                    std::atomic<size_t>& sent_bytes);

  ~tcp_stream_writer();

  detail::error init(const std::string ip, const uint16_t port);

  /// Starts this worker.
  void start();

  /// Stops this worker.
  void stop();

  /// Blocks until the worker has fully stopped running.
  void join();

private:
  /// Reads up to 2096 bytes from the socket.
  void read();

  /// writes up to 2096 bytes from the socket.
  void write();

  net::tcp_stream_socket handle_;

  bool running_;
  std::thread reader_thread_;
  std::thread writer_thread_;

  // local store of the performance metrics
  size_t received_bytes_;
  size_t sent_bytes_;

  // Store the global performance metrics of the benchmark
  std::atomic<size_t>& glob_received_bytes_;
  std::atomic<size_t>& glob_sent_bytes_;
};
#

} // namespace benchmark
