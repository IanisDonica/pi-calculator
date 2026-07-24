#pragma once

#include <cstdint>
#include <string>

namespace pi_calc {

/// decimal digits gained per extra Chudnovsky term
constexpr long double kDigitsPerChudnovskyTerm = 14.1816474627254776555L;

/// which BigInt/BigFloat implementation to compute with
enum class Backend { Library, Custom };

/// backend and thread count for a single compute_pi_* / compute_sqrt_pi_* call
struct ComputeOptions {
  Backend backend = Backend::Library;
  unsigned int threads = 1;
};

/// terms needed to safely land on decimal_places correct digits
std::uint64_t estimate_terms_for_digits(std::uint64_t decimal_places);
/// decimal digits guaranteed correct after this many terms
std::uint64_t conservative_digits_for_terms(std::uint64_t terms);
/// computes pi from a Chudnovsky term count
std::string compute_pi_terms(std::uint64_t terms, ComputeOptions options = {});
/// computes pi to a requested number of decimal digits
std::string compute_pi_digits(std::uint64_t decimal_places,
                              ComputeOptions options = {});
/// same as compute_pi_terms, but returns sqrt(pi) instead
std::string compute_sqrt_pi_terms(std::uint64_t terms,
                                  ComputeOptions options = {});
/// same as compute_pi_digits, but returns sqrt(pi) instead
std::string compute_sqrt_pi_digits(std::uint64_t decimal_places,
                                   ComputeOptions options = {});

} // namespace pi_calc
