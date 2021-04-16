/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 *  @date 06.04.2021
 */

#include "net/operation.hpp"

#include <bitset>

namespace net {

std::string to_string(operation op) {
  switch (op) {
    case operation::none:
      return "none";
    case operation::read:
      return "read";
    case operation::write:
      return "write";
    case operation::read_write:
      return "read_write";
    default: {
      std::bitset<32> x(static_cast<uint32_t>(op));
      return "unknown operation: " + x.to_string();
    }
  }
}

} // namespace net
