#pragma once

#include "pi/custom_bigint.hpp"
#include <cstdint>
#include <string>

namespace pi_calc::custom {

/// fixed-point decimal: value == scaled_value() / 10^scale(), backed by
/// custom::BigInt
class BigFloat {
public:
  BigFloat();

  /// value, at the given number of decimal places
  static BigFloat from_integer(const BigInt &value, std::uint64_t scale);
  /// scaled_value already holds value * 10^scale
  static BigFloat from_scaled(BigInt scaled_value, std::uint64_t scale);
  /// sqrt(value), to scale decimal places
  static BigFloat sqrt_integer(unsigned int value, std::uint64_t scale);

  [[nodiscard]] const BigInt &scaled_value() const;
  [[nodiscard]] std::uint64_t scale() const;

  /// square root of this value, at the same scale
  [[nodiscard]] BigFloat sqrt() const;
  [[nodiscard]] BigFloat multiply_integer(const BigInt &value) const;
  [[nodiscard]] BigFloat divide_integer(const BigInt &value) const;
  /// same value, re-expressed at new_scale decimal places
  [[nodiscard]] BigFloat rescale(std::uint64_t new_scale) const;
  /// decimal string with exactly decimal_places digits after the point
  [[nodiscard]] std::string to_string(std::uint64_t decimal_places) const;

private:
  BigInt scaled_value_;
  std::uint64_t scale_ = 0;
};

/// renders scaled_value as a decimal string with decimal_places digits after
/// the point
std::string format_scaled_decimal(const BigInt &scaled_value,
                                  std::uint64_t decimal_places);

} // namespace pi_calc::custom
