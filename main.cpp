/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 *  @date 25.03.2021
 */

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <variant>

#include "benchmark/tcp_stream_writer.hpp"
#include "detail/error.hpp"
#include "detail/socket_guard.hpp"
#include "net/multiplexer.hpp"
#include "net/socket.hpp"
#include "net/socket_manager_factory.hpp"
#include "net/socket_manager_impl.hpp"
#include "net/stream_socket.hpp"
#include "net/tcp_accept_socket.hpp"
#include "net/tcp_stream_socket.hpp"

struct dummy_factory : public net::socket_manager_factory {
  dummy_factory(std::atomic<size_t>& received_bytes,
                std::atomic<size_t>& sent_bytes,
                std::atomic<size_t>& num_handled_events)
    : received_bytes_(received_bytes),
      sent_bytes_(sent_bytes),
      num_handled_events_(num_handled_events) {
    // nop
  }

  net::socket_manager_ptr make(net::socket handle,
                               net::multiplexer* mpx) override {
    std::cout << "factory created new socket manager" << std::endl;
    return std::make_shared<net::socket_manager_impl>(
      handle, mpx, true, received_bytes_, sent_bytes_, num_handled_events_);
  }

private:
  std::atomic<size_t>& received_bytes_;
  std::atomic<size_t>& sent_bytes_;
  std::atomic<size_t>& num_handled_events_;
};

template <class What>
[[noreturn]] void exit(What what) {
  std::cerr << what << std::endl;
  abort();
}

void test_connect() {
  auto res = net::make_tcp_accept_socket(0);
  if (auto err = std::get_if<detail::error>(&res))
    exit(*err);
  auto accept_socket_pair = std::get<net::acceptor_pair>(res);
  auto accept_socket_guard
    = detail::make_socket_guard(accept_socket_pair.first);
  std::cerr << "socket is bound to port: " << accept_socket_pair.second
            << std::endl;
  if (accept_socket_guard == net::invalid_socket)
    exit("failed to create accept socket");
  std::cerr << "waiting for an incoming connection" << std::endl;
  auto guard = detail::make_socket_guard(net::accept(*accept_socket_guard));
  std::cerr << "accepted!" << std::endl;
  if (guard == net::invalid_socket)
    exit("accepted invalid socket");
  std::cerr << "all fine!" << std::endl;
}

void run_server() {
  net::multiplexer mpx;
  std::atomic<size_t> received_bytes;
  std::atomic<size_t> sent_bytes;
  std::atomic<size_t> num_handled_events;
  auto factory = std::make_shared<dummy_factory>(received_bytes, sent_bytes,
                                                 num_handled_events);
  if (auto err = mpx.init(std::move(factory)))
    exit(err);
  mpx.start();

  std::cout << "waiting for user input" << std::endl;
  std::string dummy;
  std::getline(std::cin, dummy);

  mpx.shutdown();
  mpx.join();
  std::cout << std::endl << "=========== SUMMARY =============" << std::endl;
  std::cout << "num_handled_events: " << num_handled_events << " events"
            << std::endl;
  std::cout << "sent_bytes: " << sent_bytes << " bytes" << std::endl;
  std::cout << "received_bytes: " << received_bytes << " bytes" << std::endl;
}

void run_client(std::string host, uint16_t port) {
  std::atomic<size_t> received_bytes;
  std::atomic<size_t> sent_bytes;

  std::vector<benchmark::tcp_stream_writer_ptr> writers;
  for (int i = 0; i < 10; ++i) {
    writers.emplace_back(std::make_shared<benchmark::tcp_stream_writer>(
      received_bytes, sent_bytes));
    if (auto err = writers.back()->init(host, port))
      exit(err);
  }
  for (auto& writer : writers)
    writer->start();

  // std::string dummy;
  // std::getline(std::cin, dummy);
  std::this_thread::sleep_for(std::chrono::seconds(10));
  std::cout << "woke up!" << std::endl;
  for (auto& writer : writers)
    writer->stop();
  for (auto& writer : writers)
    writer->join();

  writers.clear();
  std::cout << std::endl << "=========== SUMMARY =============" << std::endl;
  std::cout << "sent_bytes: " << sent_bytes << " bytes" << std::endl;
  std::cout << "received_bytes: " << received_bytes << " bytes" << std::endl;
}

int main(int argc, char** argv) {
  bool is_server = true;
  std::string host = "";
  uint16_t port = 0;

  for (int i = 0; i < argc; ++i) {
    switch (argv[i][1]) {
      case 's':
        is_server = true;
        break;
      case 'c':
        is_server = false;
        break;
      case 'p':
        port = std::stoi(std::string(argv[++i]));
        break;
      case 'h':
        host = std::string(argv[++i]);
        break;
      default:
        break;
    }
  }
  if (is_server)
    run_server();
  else
    run_client(host, port);
}
