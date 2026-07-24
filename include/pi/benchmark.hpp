#pragma once

#include "pi/chudnovsky.hpp"
#include <cstdint>

namespace pi_calc {

/// Times compute_pi_terms/compute_pi_digits across a sweep of thread counts
/// (1, 2, 4, ... up to options.threads, or hardware concurrency if threads ==
/// 1), printing per-thread-count timing, speedup over the single-threaded run,
/// and flagging any thread count whose result disagrees with the baseline.
///
/// @param by_terms use compute_pi_terms/compute_sqrt_pi_terms if true,
/// otherwise the digits versions
/// @param value the term count or decimal digit count, depending on by_terms
/// @param options backend to use; options.threads sets the top of the sweep
void run_benchmark(bool by_terms, std::uint64_t value, ComputeOptions options);

} // namespace pi_calc
