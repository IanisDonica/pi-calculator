#pragma once

#include <concepts>

namespace pi_calc::detail {

/// arbitrary-precision integer types the Chudnovsky/pow kernels can run on:
/// constructible from int/unsigned long/decimal-string literals, and supporting
/// the arithmetic both kernels use. Satisfied by both
/// boost::multiprecision::cpp_int and pi_calc::custom::BigInt.
template <typename Int>
concept BigIntLike =
    std::regular<Int> &&
    requires(Int value, int i, unsigned long u, const char *text) {
      Int{i};
      Int{u};
      Int{text};
      { value + value } -> std::convertible_to<Int>;
      { value - value } -> std::convertible_to<Int>;
      { value * value } -> std::convertible_to<Int>;
      { -value } -> std::convertible_to<Int>;
      { value *= value };
    };

} // namespace pi_calc::detail
