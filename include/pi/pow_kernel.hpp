#pragma once

#include "pi/big_int_concept.hpp"
#include <cstdint>

namespace pi_calc::detail {

/// base^exponent by squaring, shared by both backends' BigInt types
template <BigIntLike Int> Int pow_uint(Int base, std::uint64_t exponent) {
  Int result = 1;

  while (exponent > 0) {
    if ((exponent & 1U) != 0U)
      result *= base;

    exponent >>= 1U;
    if (exponent > 0)
      base *= base;
  }

  return result;
}

/// 10^exponent, shared by both backends' BigInt types
template <BigIntLike Int> Int pow10(std::uint64_t exponent) {
  return pow_uint<Int>(Int{10}, exponent);
}

} // namespace pi_calc::detail
