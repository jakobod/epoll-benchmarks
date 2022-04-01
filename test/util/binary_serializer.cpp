/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 */

#include <gtest/gtest.h>

#include "util/binary_serializer.hpp"

#include "fwd.hpp"

#include <algorithm>

using namespace util;

using namespace std::string_literals;

namespace {

struct dummy_class {
  std::string s_;
  std::uint8_t u8_;
  std::uint16_t u16_;
  std::uint32_t u32_;
  std::uint64_t u64_;
  std::int8_t i8_;
  std::int16_t i16_;
  std::int32_t i32_;
  std::int64_t i64_;
  float f_;
  double d_;
};

template <class Visitor>
auto visit(dummy_class& d, Visitor& f) {
  return f(d.s_, d.u8_, d.u16_, d.u32_, d.u64_, d.i8_, d.i16_, d.i32_, d.i64_,
           d.f_, d.d_);
}

#define check_serializing(expected, ...)                                       \
  do {                                                                         \
    byte_buffer buf;                                                           \
    binary_serializer serializer{buf};                                         \
    serializer(__VA_ARGS__);                                                   \
    ASSERT_EQ(buf.size(), expected.size());                                    \
    ASSERT_TRUE(std::equal(buf.begin(), buf.end(), expected.begin()));         \
  } while (false)

} // namespace

TEST(binary_serializer, empty_call) {
  byte_buffer buf;
  binary_serializer serializer{buf};
  serializer();
  ASSERT_TRUE(buf.empty());
}

TEST(binary_serializer, integer) {
  static constexpr const auto expected_result
    = make_byte_array(0x01, 0x02, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x06, 0x00,
                      0x07, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00);
  check_serializing(expected_result, std::uint8_t{1}, std::uint16_t{2},
                    std::uint32_t{3}, std::uint64_t{4}, std::int8_t{5},
                    std::int16_t{6}, std::int32_t{7}, std::int64_t{8});
}

TEST(binary_serializer, byte) {
  static constexpr const auto expected_result = make_byte_array(0x2A, 0x45);
  check_serializing(expected_result, std::byte{42}, std::byte{69});
}

TEST(binary_serializer, floats) {
  static constexpr const auto expected_result = make_byte_array(
    0xd7, 0xa3, 0x70, 0x3d, 0xa, 0x4b, 0x7a, 0x40, 0xa, 0xd7, 0x8a, 0x42);
  check_serializing(expected_result, double{420.69}, float{69.42});
}

TEST(binary_serializer, string) {
  {
    static constexpr const auto expected_result
      = make_byte_array(0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 'H',
                        'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd');
    check_serializing(expected_result, "Hello World"s);
  }
  {
    static constexpr const auto expected_result
      = make_byte_array(0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 'H',
                        'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', 0x05,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 'W', 'o', 'r',
                        'l', 'd', 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 'H', 'e', 'l', 'l', 'o');
    check_serializing(expected_result, "Hello World"s, "World"s, "Hello"s);
  }
}

TEST(binary_serializer, pair) {
  static constexpr const auto expected_result = make_byte_array(
    0xA4, 0x01, 0x00, 0x00, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  check_serializing(expected_result,
                    std::make_pair(std::uint32_t{420}, std::uint64_t{69}));
}

TEST(binary_serializer, tuple) {
  static constexpr const auto expected_result
    = make_byte_array(0xA4, 0x01, 0x00, 0x00, 0x45, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x2A, 0x39, 0x05);
  check_serializing(expected_result,
                    std::make_tuple(std::uint32_t{420}, std::uint64_t{69},
                                    std::uint8_t{42}, std::uint16_t{1337}));
}

TEST(serializer_tests, visit) {
  static constexpr const auto expected_result = make_byte_array(
    0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 'H', 'e', 'l', 'l', 'o',
    0x01, 0x02, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x05, 0x06, 0x00, 0x07, 0x00, 0x00, 0x00, 0x08, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x24, 0x40,

    0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 'W', 'o', 'r', 'l', 'd',
    0x0B, 0x0C, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0F, 0x10, 0x00, 0x11, 0x00, 0x00, 0x00, 0x12, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x98, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x34, 0x40);
  check_serializing(
    expected_result, dummy_class{"Hello", 1, 2, 3, 4, 5, 6, 7, 8, 9.0, 10.0},
    dummy_class{"World", 11, 12, 13, 14, 15, 16, 17, 18, 19.0, 20.0});
}

TEST(binary_serializer, c_style_array) {
  static constexpr const std::uint64_t input[10] = {0, 1, 2, 3, 4,
                                                    5, 6, 7, 8, 9};
  static constexpr const auto expected_result = make_byte_array(
    0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00);

  byte_buffer buf;
  binary_serializer serializer{buf};
  serializer(input);
  ASSERT_EQ(buf.size(), expected_result.size());
  ASSERT_TRUE(std::equal(buf.begin(), buf.end(), expected_result.begin()));
}

TEST(binary_serializer, vector) {
  static constexpr const std::array<std::uint64_t, 10> input{0, 1, 2, 3, 4,
                                                             5, 6, 7, 8, 9};
  static constexpr const auto expected_result = make_byte_array(
    0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00);

  byte_buffer buf;
  binary_serializer serializer{buf};
  serializer(input);
  ASSERT_EQ(buf.size(), expected_result.size());
  ASSERT_TRUE(std::equal(buf.begin(), buf.end(), expected_result.begin()));
}
