#include "pi/chudnovsky.hpp"
#include "pi/binary_split.hpp"
#include "pi/chudnovsky_kernel.hpp"
#include "pi/custom_bigfloat.hpp"
#include "pi/formatting.hpp"
#include "pi/integer_math.hpp"
#include <cmath>
#include <stdexcept>

namespace pi_calc {
namespace {

constexpr int kMultiplier = 426880;
constexpr int kSqrtTerm = 10005;
constexpr std::uint64_t kCustomGuardDigits = 10;

void validate_compute_inputs(std::uint64_t terms,
                             const ComputeOptions &options) {
  if (terms == 0)
    throw std::invalid_argument("at least one Chudnovsky term is required");

  if (options.threads == 0)
    throw std::invalid_argument("thread count must be at least one");
}

std::string compute_pi_library(std::uint64_t terms,
                               std::uint64_t decimal_places,
                               const ComputeOptions &options, bool want_sqrt) {
  const BigInt scale = pow10(decimal_places);
  const BigInt scaled_sqrt = integer_sqrt(BigInt{kSqrtTerm} * scale * scale);
  const SplitResult split = binary_split_parallel(0, terms, options.threads);
  BigInt scaled_pi = (BigInt{kMultiplier} * scaled_sqrt * split.Q) / split.P;

  if (want_sqrt)
    scaled_pi = integer_sqrt(scaled_pi * scale * scale * scale) / scale;

  return format_scaled_decimal(scaled_pi, decimal_places);
}

std::string compute_pi_custom(std::uint64_t terms, std::uint64_t decimal_places,
                              const ComputeOptions &options, bool want_sqrt) {
  using CustomInt = custom::BigInt;

  const auto split =
      detail::binary_split_parallel<CustomInt>(0, terms, options.threads);
  const std::uint64_t guarded_places = decimal_places + kCustomGuardDigits;
  const custom::BigFloat sqrt_term =
      custom::BigFloat::sqrt_integer(kSqrtTerm, guarded_places);

  custom::BigFloat pi = sqrt_term.multiply_integer(CustomInt{kMultiplier})
                            .multiply_integer(split.Q)
                            .divide_integer(split.P);

  if (want_sqrt)
    pi = pi.sqrt();

  return pi.to_string(decimal_places);
}

std::string compute_pi(std::uint64_t terms, std::uint64_t decimal_places,
                       const ComputeOptions &options, bool want_sqrt) {
  validate_compute_inputs(terms, options);

  switch (options.backend) {
  case Backend::Library:
    return compute_pi_library(terms, decimal_places, options, want_sqrt);
  case Backend::Custom:
    return compute_pi_custom(terms, decimal_places, options, want_sqrt);
  }

  throw std::invalid_argument("unknown pi computation backend");
}

} // namespace

std::uint64_t estimate_terms_for_digits(std::uint64_t decimal_places) {
  if (decimal_places == 0)
    return 1;

  return static_cast<std::uint64_t>(
             std::ceil((static_cast<long double>(decimal_places) + 1.0L) /
                       kDigitsPerChudnovskyTerm)) +
         1;
}

std::uint64_t conservative_digits_for_terms(std::uint64_t terms) {
  if (terms == 0)
    return 0;

  const auto estimated_digits = static_cast<std::uint64_t>(
      std::floor(static_cast<long double>(terms) * kDigitsPerChudnovskyTerm));

  return estimated_digits > 1 ? estimated_digits - 1 : 1;
}

std::string compute_pi_terms(std::uint64_t terms, ComputeOptions options) {
  return compute_pi(terms, conservative_digits_for_terms(terms), options,
                    false);
}

std::string compute_pi_digits(std::uint64_t decimal_places,
                              ComputeOptions options) {
  return compute_pi(estimate_terms_for_digits(decimal_places), decimal_places,
                    options, false);
}

std::string compute_sqrt_pi_terms(std::uint64_t terms, ComputeOptions options) {
  return compute_pi(terms, conservative_digits_for_terms(terms), options, true);
}

std::string compute_sqrt_pi_digits(std::uint64_t decimal_places,
                                   ComputeOptions options) {
  return compute_pi(estimate_terms_for_digits(decimal_places), decimal_places,
                    options, true);
}

} // namespace pi_calc
