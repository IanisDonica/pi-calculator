#pragma once

#include "pi/big_int_concept.hpp"
#include <cstdint>
#include <future>
#include <stdexcept>

namespace pi_calc::detail {

/// P(a,b), Q(a,b) and the step-ratio accumulator for a Chudnovsky term range
template <BigIntLike Int> struct SplitResultT {
  Int ratio; // ratio is a step-ratio accumulator
  Int Q;
  Int P;
};

/// merges two adjacent SplitResultT ranges into one, this is where the actual
/// big-integer multiplication happens, and it always runs on a single thread
template <BigIntLike Int>
SplitResultT<Int> combine_split_results(const SplitResultT<Int> &left,
                                        const SplitResultT<Int> &right) {
  return {
      .ratio = left.ratio * right.ratio,
      .Q = left.Q * right.Q,
      .P = left.P * right.Q + left.ratio * right.P,
  };
}

/// T(k), the single Chudnovsky term at index k, as a P/Q pair
template <BigIntLike Int> SplitResultT<Int> binary_split_leaf(std::uint64_t k) {
  constexpr int kA = 13591409;
  constexpr int kB = 545140134;

  const Int c3_over_24{"10939058860032000"};

  if (k == 0)
    return {.ratio = Int{1}, .Q = Int{1}, .P = Int{kA}};

  const Int term{static_cast<unsigned long>(k)};
  SplitResultT<Int> result{
      .ratio = (Int{6} * term - Int{5}) * (Int{2} * term - Int{1}) *
               (Int{6} * term - Int{1}),
      .Q = term * term * term * c3_over_24,
      .P = Int{0},
  };

  result.P = result.ratio * (Int{kA} + Int{kB} * term);

  if ((k & 1U) != 0U)
    result.P = -result.P;

  return result;
}

/// recursively splits [a, b) in half and combines the two halves, no threading
template <BigIntLike Int>
SplitResultT<Int> binary_split_serial(std::uint64_t a, std::uint64_t b) {
  if (a >= b)
    throw std::invalid_argument("binary_split requires a < b");

  if (b - a == 1)
    return binary_split_leaf<Int>(a);

  const std::uint64_t m = a + (b - a) / 2;
  const SplitResultT<Int> left = binary_split_serial<Int>(a, m);
  const SplitResultT<Int> right = binary_split_serial<Int>(m, b);
  return combine_split_results(left, right);
}

/// same split as binary_split_serial, but hands the left half to std::async and
/// halves the thread budget each level down, falling back to serial once
/// threads run out or the range gets smaller than min_terms. Only the split
/// itself is parallel, combine_split_results still runs single threaded either
/// way.
template <BigIntLike Int>
SplitResultT<Int> binary_split_parallel(std::uint64_t a, std::uint64_t b,
                                        unsigned int threads,
                                        std::uint64_t min_terms = 32) {
  if (a >= b)
    throw std::invalid_argument("binary_split requires a < b");

  if (threads <= 1 || b - a <= min_terms)
    return binary_split_serial<Int>(a, b);

  const std::uint64_t m = a + (b - a) / 2;
  const unsigned int left_threads = threads / 2;
  const unsigned int right_threads = threads - left_threads;

  auto left = std::async(std::launch::async, [=] {
    return binary_split_parallel<Int>(a, m, left_threads, min_terms);
  });
  const SplitResultT<Int> right =
      binary_split_parallel<Int>(m, b, right_threads, min_terms);

  return combine_split_results(left.get(), right);
}

} // namespace pi_calc::detail
