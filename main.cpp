#include "pi/benchmark.hpp"
#include "pi/chudnovsky.hpp"
#include <charconv>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <string_view>

namespace {

enum class Mode { Terms, Digits };

struct Options {
  Mode mode = Mode::Terms;
  std::uint64_t value = 0;
  pi_calc::ComputeOptions compute_options;
  bool benchmark = false;
  bool sqrt = false;
};

std::optional<std::uint64_t> parse_size(std::string_view text) {
  std::uint64_t value = 0;
  const auto *begin = text.data();
  const auto *end = begin + text.size();
  const auto [ptr, error] = std::from_chars(begin, end, value);

  if (error != std::errc{} || ptr != end)
    return std::nullopt;

  return value;
}

std::optional<pi_calc::Backend> parse_backend(std::string_view text) {
  if (text == "library")
    return pi_calc::Backend::Library;

  if (text == "custom")
    return pi_calc::Backend::Custom;

  return std::nullopt;
}

void print_usage(const char *program) {
  std::cerr << "Usage:\n"
            << "  " << program
            << " --terms <terms> [--backend library|custom] [--threads N] "
               "[--sqrt] [--benchmark]\n"
            << "  " << program
            << " --digits <decimal-places> [--backend library|custom] "
               "[--threads N] [--sqrt] [--benchmark]\n"
            << "\n"
            << "With no arguments, the program reads the number of terms from "
               "stdin.\n"
            << "--sqrt prints the square root of pi instead of pi, to the same "
               "precision.\n"
            << "--benchmark times the computation across a sweep of thread "
               "counts up to N (default: hardware concurrency).\n";
}

std::optional<Options> parse_args(int argc, char **argv) {
  if (argc == 1) {
    std::uint64_t terms = 0;
    if (!(std::cin >> terms))
      return std::nullopt;
    return Options{.mode = Mode::Terms, .value = terms};
  }

  Options options;
  bool has_value = false;

  for (int index = 1; index < argc; ++index) {
    const std::string_view argument = argv[index];

    if (argument == "--terms" || argument == "--digits") {
      if (has_value || index + 1 >= argc)
        return std::nullopt;

      const auto value = parse_size(argv[++index]);
      if (!value)
        return std::nullopt;

      options.mode = argument == "--terms" ? Mode::Terms : Mode::Digits;
      options.value = *value;
      has_value = true;
      continue;
    }

    if (argument == "--backend") {
      if (index + 1 >= argc)
        return std::nullopt;

      const auto backend = parse_backend(argv[++index]);
      if (!backend)
        return std::nullopt;

      options.compute_options.backend = *backend;
      continue;
    }

    if (argument == "--benchmark") {
      options.benchmark = true;
      continue;
    }

    if (argument == "--sqrt") {
      options.sqrt = true;
      continue;
    }

    if (argument == "--threads") {
      if (index + 1 >= argc)
        return std::nullopt;

      const auto threads = parse_size(argv[++index]);
      if (!threads || *threads == 0 ||
          *threads > std::numeric_limits<unsigned int>::max())
        return std::nullopt;

      options.compute_options.threads = static_cast<unsigned int>(*threads);
      continue;
    }

    return std::nullopt;
  }

  if (!has_value)
    return std::nullopt;

  return options;
}

} // namespace

int main(int argc, char **argv) {
  try {
    const auto options = parse_args(argc, argv);
    if (!options) {
      print_usage(argv[0]);
      return 1;
    }

    if (options->benchmark) {
      pi_calc::run_benchmark(options->mode == Mode::Terms, options->value,
                             options->compute_options);
      return 0;
    }

    if (options->mode == Mode::Terms && !options->sqrt)
      std::cout << pi_calc::compute_pi_terms(options->value,
                                             options->compute_options)
                << '\n';
    else if (options->mode == Mode::Terms)
      std::cout << pi_calc::compute_sqrt_pi_terms(options->value,
                                                  options->compute_options)
                << '\n';
    else if (!options->sqrt)
      std::cout << pi_calc::compute_pi_digits(options->value,
                                              options->compute_options)
                << '\n';
    else
      std::cout << pi_calc::compute_sqrt_pi_digits(options->value,
                                                   options->compute_options)
                << '\n';

    return 0;
  } catch (const std::exception &error) {
    std::cerr << "error: " << error.what() << '\n';
    return 1;
  }
}
