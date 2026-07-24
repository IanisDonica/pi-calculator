#include "pi/integer_math.hpp"
#include "pi/pow_kernel.hpp"
#include <boost/multiprecision/integer.hpp>
#include <cstdint>
#include <stdexcept>
#include <utility>

namespace pi_calc {
namespace {

std::uint64_t bit_width(const BigInt &value) {
  return boost::multiprecision::msb(value) + 1;
}

} // namespace

BigInt pow_uint(BigInt base, std::uint64_t exponent) {
  return detail::pow_uint<BigInt>(std::move(base), exponent);
}

BigInt pow10(std::uint64_t exponent) { return detail::pow10<BigInt>(exponent); }

// https://en.wikipedia.org/wiki/Integer_square_root
BigInt integer_sqrt(const BigInt &value) {
  // we actually never call integer_sqrt() with value less 0, but good practice
  // ( I think )
  if (value < 0)
    throw std::invalid_argument("integer_sqrt is undefined for negative input");

  if (value <
      2) // value 2 throws std::domain_error("No bits were set in the operand.")
    return value;

  const auto bit_count = bit_width(value);
  BigInt x = BigInt{1} << ((bit_count + 1) / 2);

  while (true) {
    BigInt next = (x + value / x) >> 1;
    if (next >= x) {
      while ((x + 1) * (x + 1) <= value) {
        ++x;
      }
      while (x * x > value) {
        --x;
      }
      return x;
    }

    x = next;
  }
}

} // namespace pi_calc
