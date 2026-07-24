cmake_minimum_required(VERSION 3.25...4.2)

project(
  pi
  VERSION 0.0.0
  LANGUAGES CXX C)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include(CTest)

set(PROJECT_CXX_STANDARD
    23
    CACHE STRING "C++ standard, available option: 20, 23")
set_property(CACHE PROJECT_CXX_STANDARD PROPERTY STRINGS 20 23)

option(BUILD_REFERENCE "Build the reference code (gmp-chudnovsky)" OFF)
find_package(Threads REQUIRED)

if(BUILD_REFERENCE)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GMP REQUIRED IMPORTED_TARGET gmp)

  file(DOWNLOAD https://gmplib.org/download/misc/gmp-chudnovsky.c
       "${CMAKE_CURRENT_BINARY_DIR}/gmp-chudnovsky.c" SHOW_PROGRESS)

  # gmp-chudnovsky.c as downloaded doesn't compile on a recent glibc: it
  # declares a local/global variable called `fmul`, which collides with
  # glibc's own `fmul` narrowing-math declaration. Rename that one variable
  # so BUILD_REFERENCE=ON works out of the box instead of requiring a manual
  # edit of a downloaded file every time the build directory is recreated.
  file(READ "${CMAKE_CURRENT_BINARY_DIR}/gmp-chudnovsky.c" GMP_CHUDNOVSKY_SRC)
  string(REPLACE "fmul" "fac_mul_tmp" GMP_CHUDNOVSKY_SRC "${GMP_CHUDNOVSKY_SRC}")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gmp-chudnovsky.c" "${GMP_CHUDNOVSKY_SRC}")

  add_executable(gmp-chudnovsky "${CMAKE_CURRENT_BINARY_DIR}/gmp-chudnovsky.c")
  target_include_directories(gmp-chudnovsky PRIVATE ${GMP_INCLUDE_DIRS})
  target_link_libraries(gmp-chudnovsky PRIVATE PkgConfig::GMP m)
  #[[
	gmp-chudnovsky <digits> <1 if you want to print pi>
	gmp-chudnovsky 100 1
	gmp-chudnovsky 1000 1
	]]
endif()

add_library(
  pi_core STATIC
  src/benchmark.cpp
  src/binary_split.cpp
  src/chudnovsky.cpp
  src/custom_bigfloat.cpp
  src/custom_bigint.cpp
  src/formatting.cpp
  src/integer_math.cpp)
target_compile_features(pi_core PUBLIC "cxx_std_${PROJECT_CXX_STANDARD}")
target_include_directories(pi_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(pi_core PUBLIC Threads::Threads)

add_executable(pi main.cpp)
target_link_libraries(pi PRIVATE pi_core)

if(BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1)
  FetchContent_MakeAvailable(Catch2)

  add_executable(pi_tests tests/pi_tests.cpp)
  target_link_libraries(pi_tests PRIVATE pi_core Catch2::Catch2WithMain)

  list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
  include(Catch)
  catch_discover_tests(pi_tests)

  add_test(NAME pi_cli_digits COMMAND pi --digits 10)
  set_tests_properties(pi_cli_digits PROPERTIES
                       PASS_REGULAR_EXPRESSION "3\\.1415926535")

  add_test(NAME pi_cli_terms COMMAND pi --terms 1)
  set_tests_properties(pi_cli_terms PROPERTIES
                       PASS_REGULAR_EXPRESSION "3\\.1415926535897")

  add_test(NAME pi_cli_rejects_invalid COMMAND pi --digits abc)
  set_tests_properties(pi_cli_rejects_invalid PROPERTIES WILL_FAIL TRUE)

  add_test(NAME pi_cli_custom_digits COMMAND pi --digits 10 --backend custom)
  set_tests_properties(pi_cli_custom_digits PROPERTIES
                       PASS_REGULAR_EXPRESSION "3\\.1415926535")

  add_test(NAME pi_cli_parallel_digits COMMAND pi --digits 100 --threads 4)
  set_tests_properties(
    pi_cli_parallel_digits
    PROPERTIES PASS_REGULAR_EXPRESSION
               "3\\.14159265358979323846264338327950288419716939937510")

  add_test(NAME pi_cli_sqrt_digits COMMAND pi --digits 20 --sqrt)
  set_tests_properties(pi_cli_sqrt_digits PROPERTIES
                       PASS_REGULAR_EXPRESSION "1\\.77245385090551602729")
endif()
