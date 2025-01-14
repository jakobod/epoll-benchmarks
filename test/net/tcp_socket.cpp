/**
 *  @author    Jakob Otto
 *  @file      tcp_socket.cpp
 *  @copyright Copyright 2023 Jakob Otto. All rights reserved.
 *             This file is part of the network-driver project, released under
 *             the GNU GPL3 License.
 */

#include "net/ip/v4_address.hpp"
#include "net/ip/v4_endpoint.hpp"
#include "net/socket/tcp_accept_socket.hpp"
#include "net/socket/tcp_stream_socket.hpp"

#include "util/error.hpp"
#include "util/error_or.hpp"

#include "net_test.hpp"

using namespace net;
using namespace net::ip;

TEST(tcp_socket_test, accept) {
  auto acc_res = make_tcp_accept_socket({v4_address::localhost, 0});
  ASSERT_EQ(util::get_error(acc_res), nullptr);
  auto acc_pair = std::get<acceptor_pair>(acc_res);
  // Connect to the accept socket
  const v4_endpoint ep{v4_address::localhost, acc_pair.second};
  auto conn_res = make_connected_tcp_stream_socket(ep);
  ASSERT_EQ(util::get_error(conn_res), nullptr);
  auto sock = std::get<tcp_stream_socket>(conn_res);
  ASSERT_NE(sock, invalid_socket);
  // Accept the connection
  auto accepted = accept(acc_pair.first);
  EXPECT_NE(accepted, invalid_socket);
  // Check functionality
  util::byte_array<10> data;
  EXPECT_EQ(write(sock, data), data.size());
  EXPECT_EQ(read(accepted, data), data.size());
}
