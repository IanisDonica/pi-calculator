#include "pi/binary_split.hpp"

namespace pi_calc {

SplitResult binary_split_parallel(std::uint64_t a, std::uint64_t b,
                                  unsigned int threads,
                                  std::uint64_t min_terms) {
  return detail::binary_split_parallel<BigInt>(a, b, threads, min_terms);
}

} // namespace pi_calc
