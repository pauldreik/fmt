#ifndef FUZZER_COMMON_H
#define FUZZER_COMMON_H

#include <cstring>     // memcpy
#include <type_traits> // trivially copyable

// Copyright (c) 2019, Paul Dreik
// License: see LICENSE.rst in the fmt root directory

// one can format to either a string, or a buf. buf is faster,
// but one may be interested in formatting to a string instead to
// verify it works as intended. to avoid a combinatoric explosion,
// select this at compile time instead of dynamically from the fuzz data
#define FMT_FUZZ_FORMAT_TO_STRING 0

// if fmt is given a buffer that is separately allocated,
// chances that address sanitizer detects out of bound reads is
// much higher. However, it slows down the fuzzing.
#define FMT_FUZZ_SEPARATE_ALLOCATION 1



// To let the the fuzzer mutation be efficient at cross pollinating
// between different types, use a fixed size format.
// The same bit pattern, interpreted as another type,
// is likely interesting.
// For this, we must know the size of the largest possible type in use.


// There are some problems on travis, claiming Nfixed is not a constant expression
// which seems to be an issue with older versions of libstdc++
#if  _GLIBCXX_RELEASE >= 7
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

namespace fmt_fuzzer {
template <typename T>
inline const char* as_chars(const T* data) {
    return static_cast<const char*>(static_cast<const void*>(data));
}
template <typename T>
inline const std::uint8_t* as_bytes(const T* data) {
    return static_cast<const std::uint8_t*>(static_cast<const void*>(data));
}


template <class Item>
inline Item assignFromBuf(const uint8_t* Data) {
// wont work in travis, disable
//#if __cplusplus >= 201402L
//  static_assert(std::is_trivially_copyable<Item>::value,
//                "Item must be blittable");
//#endif
  Item item{};
  std::memcpy(&item, Data, sizeof(Item));
  return item;
}

template <> inline bool assignFromBuf<bool>(const uint8_t* Data) {
  return !!Data[0];
}

}


#endif // FUZZER_COMMON_H
