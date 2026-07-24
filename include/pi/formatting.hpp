#pragma once

#include "pi/numeric.hpp"
#include <cstdint>
#include <string>

namespace pi_calc {

namespace detail {
/// inserts a decimal point decimal_places digits from the right, keeps the sign
/// in place
std::string insert_decimal_point(std::string digits,
                                 std::uint64_t decimal_places);
} // namespace detail

/// renders scaled_value as a decimal string with decimal_places digits after
/// the point, library backend equivalent of custom::format_scaled_decimal
std::string format_scaled_decimal(const BigInt &scaled_value,
                                  std::uint64_t decimal_places);

} // namespace pi_calc
