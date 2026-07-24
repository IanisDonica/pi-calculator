#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace pi_calc::custom {

/// arbitrary precision signed integer, stored as a sign plus a vector of
/// base-1e9 limbs
class BigInt {
public:
  BigInt();
  BigInt(int value);
  BigInt(long long value);
  BigInt(unsigned long value);
  BigInt(unsigned long long value);
  BigInt(const char *text);
  /// parses a decimal string, optionally signed
  explicit BigInt(std::string_view text);

  [[nodiscard]] bool is_zero() const;
  [[nodiscard]] std::string to_string() const;
  [[nodiscard]] BigInt abs() const;

  BigInt operator-() const;

  // BigInt is passes as a reference since it could be very very big
  BigInt &operator+=(const BigInt &rhs);
  BigInt &operator-=(const BigInt &rhs);
  BigInt &operator*=(const BigInt &rhs);
  BigInt &operator/=(const BigInt &rhs);
  BigInt &operator%=(const BigInt &rhs);

  BigInt &operator*=(std::uint32_t rhs);
  BigInt &operator/=(std::uint32_t rhs);

  friend BigInt operator+(BigInt lhs, const BigInt &rhs);
  friend BigInt operator-(BigInt lhs, const BigInt &rhs);
  friend BigInt operator*(BigInt lhs, const BigInt &rhs);
  friend BigInt operator/(const BigInt &lhs, const BigInt &rhs);
  friend BigInt operator%(const BigInt &lhs, const BigInt &rhs);

  friend BigInt operator*(BigInt lhs, std::uint32_t rhs);
  friend BigInt operator/(BigInt lhs, std::uint32_t rhs);

  friend bool operator==(const BigInt &lhs, const BigInt &rhs);
  friend bool operator<(const BigInt &lhs, const BigInt &rhs);
  friend bool operator<=(const BigInt &lhs, const BigInt &rhs);
  friend bool operator>(const BigInt &lhs, const BigInt &rhs);
  friend bool operator>=(const BigInt &lhs, const BigInt &rhs);

  // completly unrelated to shifting :3, just for easier printing
  friend std::ostream &operator<<(std::ostream &out, const BigInt &value);

private:
  static constexpr std::uint32_t kBase = 1000000000U;
  static constexpr int kBaseDigits = 9;

  std::vector<std::uint32_t> digits_;
  bool negative_ = false;

  void trim();
  void add_abs(const BigInt &rhs);
  void subtract_abs(const BigInt &rhs);
  void shift_base_add(std::uint32_t digit);

  static int compare_abs(const BigInt &lhs, const BigInt &rhs);
  static std::pair<BigInt, BigInt> divmod_abs(const BigInt &lhs,
                                              const BigInt &rhs);
};

std::string to_decimal_string(const BigInt &value);
/// base^exponent
BigInt pow_uint(BigInt base, std::uint64_t exponent);
/// 10^exponent
BigInt pow10(std::uint64_t exponent);
/// floor(sqrt(value)), throws for negative input
BigInt integer_sqrt(const BigInt &value);

} // namespace pi_calc::custom
