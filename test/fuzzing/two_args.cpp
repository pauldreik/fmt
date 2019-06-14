// Copyright (c) 2019, Paul Dreik
// License: see LICENSE.rst in the fmt root directory
#include <fmt/format.h>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

#include "fuzzer_common.h"

constexpr auto Nmax=fmt_fuzzer::Nfixed;

template <typename Item1, typename Item2>
void invoke_fmt(const uint8_t* Data, std::size_t Size) {
  constexpr auto N1 = sizeof(Item1);
  constexpr auto N2 = sizeof(Item2);
  static_assert(N1 <= Nmax, "size1 exceeded");
  static_assert(N2 <= Nmax, "size2 exceeded");
  if (Size <= Nmax + Nmax) {
    return;
  }
  Item1 item1{};
  if /*constexpr*/ (std::is_same<Item1, bool>::value) {
    item1 = !!Data[0];
  } else {
    std::memcpy(&item1, Data, N1);
  }
  Data += Nmax;
  Size -= Nmax;

  Item2 item2{};
  if /*constexpr*/ (std::is_same<Item2, bool>::value) {
    item2 = !!Data[0];
  } else {
    std::memcpy(&item2, Data, N2);
  }
  Data += Nmax;
  Size -= Nmax;

  auto fmtstring = fmt::string_view((const char*)Data, Size);

#if FMT_FUZZ_FORMAT_TO_STRING
  std::string message = fmt::format(fmtstring, item1, item2);
#else
  fmt::memory_buffer message;
  fmt::format_to(message, fmtstring, item1, item2);
#endif
}

// for dynamic dispatching to an explicit instantiation
template <typename Callback> void invoke(int index, Callback callback) {
  switch (index) {
  case 0:
    callback(bool{});
    break;
  case 1:
    callback(char{});
    break;
  case 2:
    using sc = signed char;
    callback(sc{});
    break;
  case 3:
    using uc = unsigned char;
    callback(uc{});
    break;
  case 4:
    callback(short{});
    break;
  case 5:
    using us = unsigned short;
    callback(us{});
    break;
  case 6:
    callback(int{});
    break;
  case 7:
    callback(unsigned{});
    break;
  case 8:
    callback(long{});
    break;
  case 9:
    using ul = unsigned long;
    callback(ul{});
    break;
  case 10:
    callback(float{});
    break;
  case 11:
    callback(double{});
    break;
  case 12:
    using LD = long double;
    callback(LD{});
    break;
  }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, std::size_t Size) {
  if (Size <= 3) {
    return 0;
  }

  // switch types depending on the first byte of the input
  const auto first = Data[0] & 0x0F;
  const auto second = (Data[0] & 0xF0) >> 4;
  Data++;
  Size--;

  auto outer = [=](auto param1) {
    auto inner = [=](auto param2) {
      invoke_fmt<decltype(param1), decltype(param2)>(Data, Size);
    };
    invoke(second, inner);
  };

  try {
    invoke(first, outer);
  } catch (std::exception& e) {
  }
  return 0;
}
