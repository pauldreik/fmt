#ifndef FUZZER_COMMON_H
#define FUZZER_COMMON_H
// Copyright (c) 2019, Paul Dreik
// License: see LICENSE.rst in the fmt root directory

// one can format to either a string, or a buf. buf is faster,
// but one may be interested in formatting to a string instead to
// verify it works as intended. to avoid a combinatoric explosion,
// select this at compile time instead of dynamically from the fuzz data
#define FMT_FUZZ_FORMAT_TO_STRING 1

// if fmt is given a buffer that is separately allocated,
// chances that address sanitizer detects out of bound reads is
// much higher. However, it slows down the fuzzing.
#define FMT_FUZZ_SEPARATE_ALLOCATION 1



// To let the the fuzzer mutation be efficient at cross pollinating
// between different types, use a fixed size format.
// The same bit pattern, interpreted as another type,
// is likely interesting.
// For this, we must know the size of the largest possible type in use.
// There are some problems on old compilers, hence the C++ version check
#if __cplusplus >= 201402L
# include <algorithm>
# include <cstdint>
namespace fmt_fuzzer {
  constexpr auto Nfixed = std::max(sizeof(long double), sizeof(std::intmax_t));
}
#else
namespace fmt_fuzzer {
  constexpr auto Nfixed=16;
}
#endif




#endif // FUZZER_COMMON_H
