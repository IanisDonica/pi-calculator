#pragma once

#include "pi/chudnovsky_kernel.hpp"
#include "pi/numeric.hpp"
#include <cstdint>

namespace pi_calc {

/// P/Q accumulator for the library backend (boost::multiprecision BigInt)
using SplitResult = detail::SplitResultT<BigInt>;

/// binary splits the Chudnovsky term range [a, b) using the library backend's
/// BigInt, see pi_calc::detail::binary_split_parallel for how the threading
/// works
SplitResult binary_split_parallel(std::uint64_t a, std::uint64_t b,
                                  unsigned int threads,
                                  std::uint64_t min_terms = 32);

} // namespace pi_calc
