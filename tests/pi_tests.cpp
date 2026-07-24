#include "pi/binary_split.hpp"
#include "pi/chudnovsky.hpp"
#include "pi/custom_bigfloat.hpp"
#include "pi/custom_bigint.hpp"
#include "pi/formatting.hpp"
#include "pi/integer_math.hpp"
#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <stdexcept>
#include <string>

TEST_CASE("digit estimation", "[chudnovsky]") {
  REQUIRE(pi_calc::estimate_terms_for_digits(0) == 1);
  REQUIRE(pi_calc::estimate_terms_for_digits(10) >= 1);
  REQUIRE(pi_calc::estimate_terms_for_digits(100) >
          pi_calc::estimate_terms_for_digits(10));
}

TEST_CASE("known digit outputs", "[chudnovsky]") {
  const pi_calc::ComputeOptions custom_options{pi_calc::Backend::Custom, 1};
  const pi_calc::ComputeOptions parallel_options{pi_calc::Backend::Library, 4};

  REQUIRE(pi_calc::compute_pi_digits(0) == "3");
  REQUIRE(pi_calc::compute_pi_digits(1) == "3.1");
  REQUIRE(pi_calc::compute_pi_digits(2) == "3.14");
  REQUIRE(pi_calc::compute_pi_digits(10) == "3.1415926535");
  REQUIRE(pi_calc::compute_pi_digits(10, custom_options) == "3.1415926535");

  const std::string first_100_digits =
      "3.14159265358979323846264338327950288419716939937510"
      "58209749445923078164062862089986280348253421170679";
  REQUIRE(pi_calc::compute_pi_digits(100) == first_100_digits);
  REQUIRE(pi_calc::compute_pi_digits(100, parallel_options) ==
          first_100_digits);

  const pi_calc::ComputeOptions custom_parallel_options{
      pi_calc::Backend::Custom, 4};
  REQUIRE(pi_calc::compute_pi_digits(100, custom_options) == first_100_digits);
  REQUIRE(pi_calc::compute_pi_digits(100, custom_parallel_options) ==
          first_100_digits);
}

TEST_CASE("compute_pi_terms rejects zero threads", "[chudnovsky][errors]") {
  const pi_calc::ComputeOptions zero_threads{pi_calc::Backend::Library, 0};
  REQUIRE_THROWS_AS(pi_calc::compute_pi_terms(1, zero_threads),
                    std::invalid_argument);
}

TEST_CASE("compute_pi_terms rejects unknown backend", "[chudnovsky][errors]") {
  const pi_calc::ComputeOptions bad_backend{static_cast<pi_calc::Backend>(99),
                                            1};
  REQUIRE_THROWS_AS(pi_calc::compute_pi_digits(5, bad_backend),
                    std::invalid_argument);
}

TEST_CASE("binary_split_parallel", "[chudnovsky]") {
  REQUIRE_THROWS_AS(pi_calc::binary_split_parallel(5, 5, 1),
                    std::invalid_argument);

  const auto split = pi_calc::binary_split_parallel(0, 1, 1);
  REQUIRE(split.ratio == pi_calc::BigInt{1});
  REQUIRE(split.Q == pi_calc::BigInt{1});
  REQUIRE(split.P == pi_calc::BigInt{13591409});
}

TEST_CASE("term mode", "[chudnovsky]") {
  REQUIRE(pi_calc::conservative_digits_for_terms(0) == 0);
  REQUIRE(pi_calc::conservative_digits_for_terms(1) == 13);
  REQUIRE(pi_calc::compute_pi_terms(1) == "3.1415926535897");
}

TEST_CASE("integer helpers", "[integer_math]") {
  REQUIRE(pi_calc::pow10(0) == pi_calc::BigInt{1});
  REQUIRE(pi_calc::pow10(5) == pi_calc::BigInt{100000});
  REQUIRE(pi_calc::integer_sqrt(pi_calc::BigInt{0}) == pi_calc::BigInt{0});
  REQUIRE(pi_calc::integer_sqrt(pi_calc::BigInt{1}) == pi_calc::BigInt{1});
  REQUIRE(pi_calc::integer_sqrt(pi_calc::BigInt{15}) == pi_calc::BigInt{3});
  REQUIRE(pi_calc::integer_sqrt(pi_calc::BigInt{16}) == pi_calc::BigInt{4});
  REQUIRE(pi_calc::integer_sqrt(pi_calc::BigInt{99}) == pi_calc::BigInt{9});
  REQUIRE(pi_calc::pow_uint(pi_calc::BigInt{2}, 10) == pi_calc::BigInt{1024});
  REQUIRE(pi_calc::pow_uint(pi_calc::BigInt{5}, 0) == pi_calc::BigInt{1});
}

TEST_CASE("integer_sqrt rejects negative input", "[integer_math][errors]") {
  REQUIRE_THROWS_AS(pi_calc::integer_sqrt(pi_calc::BigInt{-1}),
                    std::invalid_argument);
}

TEST_CASE("decimal formatting", "[formatting]") {
  REQUIRE(pi_calc::format_scaled_decimal(pi_calc::BigInt{3}, 0) == "3");
  REQUIRE(pi_calc::format_scaled_decimal(pi_calc::BigInt{314}, 2) == "3.14");
  REQUIRE(pi_calc::format_scaled_decimal(pi_calc::BigInt{42}, 4) == "0.0042");
  REQUIRE(pi_calc::format_scaled_decimal(pi_calc::BigInt{-42}, 4) == "-0.0042");
  REQUIRE(pi_calc::format_scaled_decimal(pi_calc::BigInt{0}, 3) == "0.000");
}

TEST_CASE("custom bigint", "[custom_bigint]") {
  using pi_calc::custom::BigInt;

  REQUIRE(BigInt{"999999999"} + BigInt{1} == BigInt{"1000000000"});
  REQUIRE(BigInt{"1000000000"} - BigInt{1} == BigInt{"999999999"});
  REQUIRE(BigInt{-5} + BigInt{2} == BigInt{-3});
  REQUIRE(BigInt{5} - BigInt{7} == BigInt{-2});
  REQUIRE(BigInt{"123456789"} * BigInt{"987654321"} ==
          BigInt{"121932631112635269"});
  REQUIRE(BigInt{"1000000000000000000"} / BigInt{"1000000000"} ==
          BigInt{"1000000000"});
  REQUIRE(BigInt{"1000000000000000001"} % BigInt{"1000000000"} == BigInt{1});

  const BigInt left{"12345678901234567890"};
  const BigInt right{"9876543210"};
  const BigInt product = left * right;
  REQUIRE(product / right == left);
  REQUIRE(product % right == BigInt{0});
  REQUIRE(pi_calc::custom::integer_sqrt(BigInt{"15241578750190521"}) ==
          BigInt{"123456789"});
}

TEST_CASE("custom bigint string parsing", "[custom_bigint]") {
  using pi_calc::custom::BigInt;

  REQUIRE(BigInt{"+123"} == BigInt{123});
  REQUIRE(BigInt{"000123"}.to_string() == "123");
  REQUIRE(BigInt{"-0"}.is_zero());
  REQUIRE(BigInt{"-0"}.to_string() == "0");

  REQUIRE_THROWS_AS(BigInt{std::string_view{""}}, std::invalid_argument);
  REQUIRE_THROWS_AS(BigInt{"-"}, std::invalid_argument);
  REQUIRE_THROWS_AS(BigInt{"12a3"}, std::invalid_argument);
}

TEST_CASE("custom bigint comparisons", "[custom_bigint]") {
  using pi_calc::custom::BigInt;

  REQUIRE(BigInt{5} < BigInt{10});
  REQUIRE(BigInt{-5} < BigInt{5});
  REQUIRE(BigInt{-10} < BigInt{-5});
  REQUIRE(BigInt{5} <= BigInt{5});
  REQUIRE(BigInt{10} > BigInt{5});
  REQUIRE(BigInt{5} >= BigInt{5});
  REQUIRE(BigInt{-5} == BigInt{-5});
  REQUIRE_FALSE(BigInt{5} == BigInt{-5});
}

TEST_CASE("custom bigint truncating division and modulo", "[custom_bigint]") {
  using pi_calc::custom::BigInt;

  REQUIRE(BigInt{-7} / BigInt{2} == BigInt{-3});
  REQUIRE(BigInt{-7} % BigInt{2} == BigInt{-1});
  REQUIRE(BigInt{7} / BigInt{-2} == BigInt{-3});
  REQUIRE(BigInt{7} % BigInt{-2} == BigInt{1});

  REQUIRE_THROWS_AS(BigInt{1} / BigInt{0}, std::domain_error);
  REQUIRE_THROWS_AS(BigInt{1} % BigInt{0}, std::domain_error);
  REQUIRE_THROWS_AS(BigInt{1} /= 0U, std::domain_error);
}

TEST_CASE("custom bigint scalar multiply and divide", "[custom_bigint]") {
  using pi_calc::custom::BigInt;

  REQUIRE(BigInt{"1000000000"} * 3U == BigInt{"3000000000"});
  REQUIRE(BigInt{"3000000000"} / 3U == BigInt{"1000000000"});
  REQUIRE(BigInt{5} * 0U == BigInt{0});
}

TEST_CASE("custom bigint to_string and stream output", "[custom_bigint]") {
  using pi_calc::custom::BigInt;

  REQUIRE(BigInt{0}.to_string() == "0");
  REQUIRE(BigInt{"-1000000000"}.to_string() == "-1000000000");

  std::ostringstream out;
  out << BigInt{"-42"};
  REQUIRE(out.str() == "-42");
}

TEST_CASE("custom bigint abs and unary minus", "[custom_bigint]") {
  using pi_calc::custom::BigInt;

  REQUIRE(BigInt{-5}.abs() == BigInt{5});
  REQUIRE(BigInt{5}.abs() == BigInt{5});
  REQUIRE(-BigInt{0} == BigInt{0});
  REQUIRE((-BigInt{0}).to_string() == "0");
}

TEST_CASE("custom pow_uint and pow10", "[custom_bigint]") {
  using pi_calc::custom::BigInt;

  REQUIRE(pi_calc::custom::pow_uint(BigInt{2}, 10) == BigInt{1024});
  REQUIRE(pi_calc::custom::pow10(0) == BigInt{1});
  REQUIRE(pi_calc::custom::pow10(3) == BigInt{1000});
}

TEST_CASE("custom integer_sqrt rejects negative input",
          "[custom_bigint][errors]") {
  REQUIRE_THROWS_AS(pi_calc::custom::integer_sqrt(pi_calc::custom::BigInt{-1}),
                    std::invalid_argument);
}

TEST_CASE("custom bigfloat", "[custom_bigfloat]") {
  using pi_calc::custom::BigFloat;
  using pi_calc::custom::BigInt;

  REQUIRE(BigFloat::from_integer(BigInt{3}, 2).to_string(2) == "3.00");
  REQUIRE(BigFloat::from_scaled(BigInt{314159}, 5).to_string(3) == "3.141");
  REQUIRE(BigFloat::sqrt_integer(2, 5).to_string(5) == "1.41421");
}

TEST_CASE("custom bigfloat rescale", "[custom_bigfloat]") {
  using pi_calc::custom::BigFloat;
  using pi_calc::custom::BigInt;

  REQUIRE(BigFloat::from_integer(BigInt{3}, 2).rescale(4).to_string(4) ==
          "3.0000");
  REQUIRE(BigFloat::from_integer(BigInt{3}, 4).rescale(2).to_string(2) ==
          "3.00");
  REQUIRE(BigFloat::from_integer(BigInt{3}, 2).rescale(2).to_string(2) ==
          "3.00");
}

TEST_CASE("custom bigfloat multiply and divide by integer",
          "[custom_bigfloat]") {
  using pi_calc::custom::BigFloat;
  using pi_calc::custom::BigInt;

  REQUIRE(BigFloat::from_integer(BigInt{2}, 2)
              .multiply_integer(BigInt{5})
              .to_string(2) == "10.00");
  REQUIRE(BigFloat::from_integer(BigInt{10}, 2)
              .divide_integer(BigInt{4})
              .to_string(2) == "2.50");
}

TEST_CASE("custom bigfloat to_string with zero decimal places",
          "[custom_bigfloat]") {
  using pi_calc::custom::BigFloat;
  using pi_calc::custom::BigInt;

  REQUIRE(BigFloat::from_integer(BigInt{7}, 2).to_string(0) == "7");
}

TEST_CASE("compute_pi_terms rejects zero terms", "[chudnovsky][errors]") {
  REQUIRE_THROWS_AS(pi_calc::compute_pi_terms(0), std::invalid_argument);
}

TEST_CASE("sqrt of pi", "[chudnovsky]") {
  const pi_calc::ComputeOptions custom_options{pi_calc::Backend::Custom, 1};
  const std::string sqrt_pi_30 = "1.772453850905516027298167483341";

  REQUIRE(pi_calc::compute_sqrt_pi_digits(30) == sqrt_pi_30);
  REQUIRE(pi_calc::compute_sqrt_pi_digits(30, custom_options) == sqrt_pi_30);
}

TEST_CASE("custom bigfloat sqrt", "[custom_bigfloat]") {
  using pi_calc::custom::BigFloat;
  using pi_calc::custom::BigInt;

  REQUIRE(BigFloat::from_integer(BigInt{2}, 5).sqrt().to_string(5) ==
          "1.41421");
}
