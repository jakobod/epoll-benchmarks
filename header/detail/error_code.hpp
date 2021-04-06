/**
 *  @author Jakob Otto
 *  @email jakob.otto@haw-hamburg.de
 *  @date 18.02.2021
 */

#pragma once

#include <string>

namespace detail {

enum error_code {
  no_error = 0,
  runtime_error,
  // socket errors
  socket_operation_failed,
  invalid_argument,
};

std::string to_string(error_code err);

} // namespace detail
