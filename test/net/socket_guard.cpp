/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 */

#include "fwd.hpp"

#include "net/socket_guard.hpp"
#include "net/stream_socket.hpp"

#include "util/error.hpp"

#include "net_test.hpp"

using namespace net;

namespace {

struct socket_guard_test : public testing::Test {
  socket_guard_test()
    : sockets{stream_socket{invalid_socket_id},
              stream_socket{invalid_socket_id}} {
    auto socket_res = make_stream_socket_pair();
    EXPECT_EQ(get_error(socket_res), nullptr);
    sockets = std::get<stream_socket_pair>(socket_res);
  }

  stream_socket_pair sockets;
};

} // namespace

TEST_F(socket_guard_test, close) {
  {
    // Should close the socket after leaving the scope
    auto guard = make_socket_guard(sockets.first);
  }
  util::byte_array<1> data;
  EXPECT_LT(write(sockets.first, data), 0);
  EXPECT_EQ(read(sockets.second, data), 0);
}

TEST_F(socket_guard_test, release) {
  {
    // Should close the socket after leaving the scope
    auto guard = make_socket_guard(sockets.first);
    auto sock = guard.release();
    EXPECT_EQ(sock, sockets.first);
  }
  util::byte_array<1> data;
  EXPECT_EQ(write(sockets.first, data), 1);
  EXPECT_EQ(read(sockets.second, data), 1);
  close(sockets.first);
  close(sockets.second);
}
