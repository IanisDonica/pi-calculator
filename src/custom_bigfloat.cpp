#include "pi/custom_bigfloat.hpp"
#include "pi/formatting.hpp"
#include <utility>

namespace pi_calc::custom {

BigFloat::BigFloat() = default;

BigFloat BigFloat::from_integer(const BigInt &value, std::uint64_t scale) {
  return BigFloat::from_scaled(value * pow10(scale), scale);
}

BigFloat BigFloat::from_scaled(BigInt scaled_value, std::uint64_t scale) {
  BigFloat result;
  result.scaled_value_ = std::move(scaled_value);
  result.scale_ = scale;
  return result;
}

BigFloat BigFloat::sqrt_integer(unsigned int value, std::uint64_t scale) {
  const BigInt factor = pow10(scale);
  return BigFloat::from_scaled(
      integer_sqrt(BigInt{static_cast<unsigned long long>(value)} * factor *
                   factor),
      scale);
}

const BigInt &BigFloat::scaled_value() const { return scaled_value_; }

std::uint64_t BigFloat::scale() const { return scale_; }

BigFloat BigFloat::sqrt() const {
  return BigFloat::from_scaled(integer_sqrt(scaled_value_ * pow10(scale_)),
                               scale_);
}

BigFloat BigFloat::multiply_integer(const BigInt &value) const {
  return BigFloat::from_scaled(scaled_value_ * value, scale_);
}

BigFloat BigFloat::divide_integer(const BigInt &value) const {
  return BigFloat::from_scaled(scaled_value_ / value, scale_);
}

BigFloat BigFloat::rescale(std::uint64_t new_scale) const {
  if (new_scale == scale_)
    return *this;

  if (new_scale > scale_)
    return BigFloat::from_scaled(scaled_value_ * pow10(new_scale - scale_),
                                 new_scale);

  return BigFloat::from_scaled(scaled_value_ / pow10(scale_ - new_scale),
                               new_scale);
}

std::string BigFloat::to_string(std::uint64_t decimal_places) const {
  return format_scaled_decimal(rescale(decimal_places).scaled_value_,
                               decimal_places);
}

std::string format_scaled_decimal(const BigInt &scaled_value,
                                  std::uint64_t decimal_places) {
  return pi_calc::detail::insert_decimal_point(scaled_value.to_string(),
                                               decimal_places);
}

} // namespace pi_calc::custom
