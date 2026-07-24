#include "pi/benchmark.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace pi_calc {
namespace {

std::vector<unsigned int> thread_sweep(unsigned int max_threads) {
  std::vector<unsigned int> threads;
  for (unsigned int count = 1; count < max_threads; count *= 2)
    threads.push_back(count);

  if (threads.empty() || threads.back() != max_threads)
    threads.push_back(max_threads);

  return threads;
}

} // namespace

void run_benchmark(bool by_terms, std::uint64_t value, ComputeOptions options) {
  const unsigned int max_threads = options.threads > 1
                                       ? options.threads
                                       : std::thread::hardware_concurrency();

  std::cout << "backend="
            << (options.backend == Backend::Library ? "library" : "custom")
            << " " << (by_terms ? "terms" : "digits") << "=" << value << '\n';

  double baseline_ms = 0.0;
  std::string baseline_result;

  for (const unsigned int threads : thread_sweep(max_threads)) {
    ComputeOptions run_options = options;
    run_options.threads = threads;

    const auto start = std::chrono::steady_clock::now();
    const std::string result = by_terms ? compute_pi_terms(value, run_options)
                                        : compute_pi_digits(value, run_options);
    const double ms = std::chrono::duration<double, std::milli>(
                          std::chrono::steady_clock::now() - start)
                          .count();

    if (threads == 1) {
      baseline_ms = ms;
      baseline_result = result;
    }

    std::cout << "  threads=" << threads << "  time=" << ms
              << "ms  speedup=" << (baseline_ms / ms) << 'x';
    if (result != baseline_result)
      std::cout << "  MISMATCH";
    std::cout << '\n';
  }
}

} // namespace pi_calc
