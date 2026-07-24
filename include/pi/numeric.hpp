#pragma once

#include <boost/multiprecision/cpp_int.hpp>
#include <string>

namespace pi_calc {

/// library backend's BigInt, just boost::multiprecision::cpp_int under a
/// shorter name
using BigInt = boost::multiprecision::cpp_int;

inline std::string to_decimal_string(const BigInt &value) {
  return value.str();
}

} // namespace pi_calc
