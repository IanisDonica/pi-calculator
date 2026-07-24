#include "pi/formatting.hpp"

namespace pi_calc {

namespace detail {

std::string insert_decimal_point(std::string digits,
                                 std::uint64_t decimal_places) {
  if (decimal_places == 0)
    return digits;

  bool negative = false;
  if (!digits.empty() && digits.front() == '-') {
    negative = true;
    digits.erase(digits.begin());
  }

  if (digits.size() <= decimal_places)
    digits.insert(0, decimal_places - digits.size() + 1, '0');

  const auto point_position = digits.size() - decimal_places;
  digits.insert(point_position, 1, '.');
  if (negative)
    digits.insert(digits.begin(), '-');

  return digits;
}

} // namespace detail

std::string format_scaled_decimal(const BigInt &scaled_value,
                                  std::uint64_t decimal_places) {
  return detail::insert_decimal_point(to_decimal_string(scaled_value),
                                      decimal_places);
}

} // namespace pi_calc
