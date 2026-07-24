#include "pi/custom_bigint.hpp"
#include "pi/pow_kernel.hpp"
#include <algorithm>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace pi_calc::custom {

BigInt::BigInt() = default;

BigInt::BigInt(int value) : BigInt(static_cast<long long>(value)) {}

BigInt::BigInt(long long value)
    : BigInt(static_cast<unsigned long long>(value < 0 ? -value : value)) {
  negative_ = value < 0;
}

BigInt::BigInt(unsigned long value)
    : BigInt(static_cast<unsigned long long>(value)) {}

BigInt::BigInt(unsigned long long value) {
  while (value > 0) {
    digits_.push_back(static_cast<std::uint32_t>(value % kBase));
    value /= kBase;
  }
}

BigInt::BigInt(const char *text) : BigInt(std::string_view{text}) {}

BigInt::BigInt(std::string_view text) {
  if (text.empty())
    throw std::invalid_argument("BigInt requires a non-empty decimal string");

  std::size_t first_digit = 0;
  if (text.front() == '-' || text.front() == '+') {
    negative_ = text.front() == '-';
    first_digit = 1;
  }

  if (first_digit == text.size())
    throw std::invalid_argument("BigInt decimal string has no digits");

  for (std::size_t end = text.size(); end > first_digit;) {
    const auto begin =
        end > first_digit + kBaseDigits ? end - kBaseDigits : first_digit;
    std::uint32_t block = 0;

    for (std::size_t index = begin; index < end; ++index) {
      const char digit = text[index];
      if (digit < '0' || digit > '9')
        throw std::invalid_argument("BigInt decimal string contains non-digit");
      block = block * 10U + static_cast<std::uint32_t>(digit - '0');
    }

    digits_.push_back(block);
    end = begin;
  }

  trim();
}

bool BigInt::is_zero() const { return digits_.empty(); }

std::string BigInt::to_string() const {
  if (is_zero())
    return "0";

  std::ostringstream out;
  if (negative_)
    out << '-';

  out << digits_.back();
  for (auto iter = digits_.rbegin() + 1; iter != digits_.rend(); ++iter) {
    out << std::setw(kBaseDigits) << std::setfill('0') << *iter;
  }

  return out.str();
}

BigInt BigInt::abs() const {
  BigInt result = *this;
  result.negative_ = false;
  return result;
}

BigInt BigInt::operator-() const {
  BigInt result = *this;
  if (!result.is_zero())
    result.negative_ = !result.negative_;
  return result;
}

BigInt &BigInt::operator+=(const BigInt &rhs) {
  if (negative_ == rhs.negative_) {
    add_abs(rhs);
    return *this;
  }

  const int comparison = compare_abs(*this, rhs);
  if (comparison >= 0) {
    subtract_abs(rhs);
  } else {
    BigInt result = rhs;
    result.subtract_abs(*this);
    *this = result;
  }

  trim();
  return *this;
}

BigInt &BigInt::operator-=(const BigInt &rhs) {
  *this += -rhs;
  return *this;
}

BigInt &BigInt::operator*=(const BigInt &rhs) {
  if (is_zero() || rhs.is_zero()) {
    digits_.clear();
    negative_ = false;
    return *this;
  }

  std::vector<std::uint32_t> result(digits_.size() + rhs.digits_.size());
  for (std::size_t left = 0; left < digits_.size(); ++left) {
    std::uint64_t carry = 0;
    for (std::size_t right = 0; right < rhs.digits_.size() || carry > 0;
         ++right) {
      const std::uint64_t product =
          static_cast<std::uint64_t>(result[left + right]) + carry +
          static_cast<std::uint64_t>(digits_[left]) *
              (right < rhs.digits_.size() ? rhs.digits_[right] : 0U);
      result[left + right] = static_cast<std::uint32_t>(product % kBase);
      carry = product / kBase;
    }
  }

  digits_ = std::move(result);
  negative_ = negative_ != rhs.negative_;
  trim();
  return *this;
}

BigInt &BigInt::operator/=(const BigInt &rhs) {
  *this = *this / rhs;
  return *this;
}

BigInt &BigInt::operator%=(const BigInt &rhs) {
  *this = *this % rhs;
  return *this;
}

BigInt &BigInt::operator*=(std::uint32_t rhs) {
  if (is_zero() || rhs == 0U) {
    digits_.clear();
    negative_ = false;
    return *this;
  }

  std::uint64_t carry = 0;
  for (auto &digit : digits_) {
    const std::uint64_t product =
        static_cast<std::uint64_t>(digit) * rhs + carry;
    digit = static_cast<std::uint32_t>(product % kBase);
    carry = product / kBase;
  }

  while (carry > 0) {
    digits_.push_back(static_cast<std::uint32_t>(carry % kBase));
    carry /= kBase;
  }

  return *this;
}

BigInt &BigInt::operator/=(std::uint32_t rhs) {
  if (rhs == 0U)
    throw std::domain_error("BigInt division by zero");

  std::uint64_t remainder = 0;
  for (std::size_t index = digits_.size(); index-- > 0;) {
    const std::uint64_t current = digits_[index] + remainder * kBase;
    digits_[index] = static_cast<std::uint32_t>(current / rhs);
    remainder = current % rhs;
  }

  trim();
  return *this;
}

BigInt operator+(BigInt lhs, const BigInt &rhs) {
  lhs += rhs;
  return lhs;
}

BigInt operator-(BigInt lhs, const BigInt &rhs) {
  lhs -= rhs;
  return lhs;
}

BigInt operator*(BigInt lhs, const BigInt &rhs) {
  lhs *= rhs;
  return lhs;
}

BigInt operator/(const BigInt &lhs, const BigInt &rhs) {
  if (rhs.is_zero())
    throw std::domain_error("BigInt division by zero");

  const auto [quotient, remainder] = BigInt::divmod_abs(lhs.abs(), rhs.abs());
  BigInt result = quotient;
  result.negative_ = lhs.negative_ != rhs.negative_;
  result.trim();
  return result;
}

BigInt operator%(const BigInt &lhs, const BigInt &rhs) {
  if (rhs.is_zero())
    throw std::domain_error("BigInt modulo by zero");

  const auto [quotient, remainder] = BigInt::divmod_abs(lhs.abs(), rhs.abs());
  (void)quotient;
  BigInt result = remainder;
  result.negative_ = lhs.negative_;
  result.trim();
  return result;
}

BigInt operator*(BigInt lhs, std::uint32_t rhs) {
  lhs *= rhs;
  return lhs;
}

BigInt operator/(BigInt lhs, std::uint32_t rhs) {
  lhs /= rhs;
  return lhs;
}

bool operator==(const BigInt &lhs, const BigInt &rhs) {
  return lhs.negative_ == rhs.negative_ && lhs.digits_ == rhs.digits_;
}

bool operator<(const BigInt &lhs, const BigInt &rhs) {
  if (lhs.negative_ != rhs.negative_)
    return lhs.negative_;

  const int comparison = BigInt::compare_abs(lhs, rhs);
  return lhs.negative_ ? comparison > 0 : comparison < 0;
}

bool operator<=(const BigInt &lhs, const BigInt &rhs) { return !(rhs < lhs); }

bool operator>(const BigInt &lhs, const BigInt &rhs) { return rhs < lhs; }

bool operator>=(const BigInt &lhs, const BigInt &rhs) { return !(lhs < rhs); }

std::ostream &operator<<(std::ostream &out, const BigInt &value) {
  return out << value.to_string();
}

void BigInt::trim() {
  while (!digits_.empty() && digits_.back() == 0U) {
    digits_.pop_back();
  }

  if (digits_.empty())
    negative_ = false;
}

void BigInt::add_abs(const BigInt &rhs) {
  const auto max_size = std::max(digits_.size(), rhs.digits_.size());
  digits_.resize(max_size);

  std::uint64_t carry = 0;
  for (std::size_t index = 0; index < max_size || carry > 0; ++index) {
    if (index == digits_.size())
      digits_.push_back(0);

    const std::uint64_t sum =
        static_cast<std::uint64_t>(digits_[index]) + carry +
        (index < rhs.digits_.size() ? rhs.digits_[index] : 0U);
    digits_[index] = static_cast<std::uint32_t>(sum % kBase);
    carry = sum / kBase;
  }
}

void BigInt::subtract_abs(const BigInt &rhs) {
  std::int64_t carry = 0;
  for (std::size_t index = 0; index < rhs.digits_.size() || carry != 0;
       ++index) {
    const std::int64_t difference =
        static_cast<std::int64_t>(digits_[index]) -
        (index < rhs.digits_.size() ? rhs.digits_[index] : 0U) - carry;

    if (difference < 0) {
      digits_[index] = static_cast<std::uint32_t>(
          difference + static_cast<std::int64_t>(kBase));
      carry = 1;
    } else {
      digits_[index] = static_cast<std::uint32_t>(difference);
      carry = 0;
    }
  }

  trim();
}

void BigInt::shift_base_add(std::uint32_t digit) {
  if (is_zero() && digit == 0U)
    return;

  digits_.insert(digits_.begin(), digit);
  trim();
}

int BigInt::compare_abs(const BigInt &lhs, const BigInt &rhs) {
  if (lhs.digits_.size() != rhs.digits_.size())
    return lhs.digits_.size() < rhs.digits_.size() ? -1 : 1;

  for (std::size_t index = lhs.digits_.size(); index-- > 0;) {
    if (lhs.digits_[index] != rhs.digits_[index])
      return lhs.digits_[index] < rhs.digits_[index] ? -1 : 1;
  }

  return 0;
}

std::pair<BigInt, BigInt> BigInt::divmod_abs(const BigInt &lhs,
                                             const BigInt &rhs) {
  if (rhs.is_zero())
    throw std::domain_error("BigInt division by zero");

  if (compare_abs(lhs, rhs) < 0)
    return {BigInt{0}, lhs};

  const auto normalization = static_cast<std::uint32_t>(
      kBase / (static_cast<std::uint64_t>(rhs.digits_.back()) + 1ULL));
  BigInt dividend = lhs;
  BigInt divisor = rhs;
  dividend *= normalization;
  divisor *= normalization;

  BigInt quotient;
  BigInt remainder;
  quotient.digits_.assign(dividend.digits_.size(), 0U);

  for (std::size_t index = dividend.digits_.size(); index-- > 0;) {
    remainder.shift_base_add(dividend.digits_[index]);

    const auto divisor_size = divisor.digits_.size();
    const std::uint64_t high = remainder.digits_.size() > divisor_size
                                   ? remainder.digits_[divisor_size]
                                   : 0U;
    const std::uint64_t next = remainder.digits_.size() > divisor_size - 1
                                   ? remainder.digits_[divisor_size - 1]
                                   : 0U;
    std::uint64_t quotient_digit =
        (kBase * high + next) / divisor.digits_.back();
    quotient_digit = std::min<std::uint64_t>(quotient_digit, kBase - 1ULL);

    BigInt candidate = divisor * static_cast<std::uint32_t>(quotient_digit);
    while (remainder < candidate) {
      --quotient_digit;
      candidate -= divisor;
    }

    remainder -= candidate;
    quotient.digits_[index] = static_cast<std::uint32_t>(quotient_digit);
  }

  quotient.trim();
  remainder /= normalization;
  remainder.trim();
  return {quotient, remainder};
}

std::string to_decimal_string(const BigInt &value) { return value.to_string(); }

BigInt pow_uint(BigInt base, std::uint64_t exponent) {
  return detail::pow_uint<BigInt>(std::move(base), exponent);
}

BigInt pow10(std::uint64_t exponent) { return detail::pow10<BigInt>(exponent); }

BigInt integer_sqrt(const BigInt &value) {
  if (value < BigInt{0})
    throw std::invalid_argument("integer_sqrt is undefined for negative input");

  if (value < BigInt{2})
    return value;

  const std::uint64_t decimal_digits =
      static_cast<std::uint64_t>(value.to_string().size());
  BigInt x = pow10((decimal_digits + 1) / 2);
  const BigInt one = 1;

  while (true) {
    BigInt next = (x + value / x) / 2U;
    if (next >= x) {
      while ((x + one) * (x + one) <= value) {
        x += one;
      }
      while (x * x > value) {
        x -= one;
      }
      return x;
    }

    x = next;
  }
}

} // namespace pi_calc::custom
