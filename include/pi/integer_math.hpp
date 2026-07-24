#pragma once

#include "pi/numeric.hpp"
#include <cstdint>

namespace pi_calc {

/// base^exponent, library backend (boost BigInt)
BigInt pow_uint(BigInt base, std::uint64_t exponent);
/// 10^exponent, library backend
BigInt pow10(std::uint64_t exponent);
/// floor(sqrt(value)), throws for negative input, library backend
BigInt integer_sqrt(const BigInt &value);

} // namespace pi_calc
