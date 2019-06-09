// Copyright (c) 2019, Paul Dreik
// License: see LICENSE.rst in the fmt root directory

#include <fmt/core.h>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <fmt/chrono.h>

template <typename Item>
void invoke_fmt(const uint8_t* Data, std::size_t Size) {
  const auto N = sizeof(Item);
  if (Size <= N) {
    return;
  }
  Item item{};
  if /*constexpr*/ (std::is_same<Item, bool>::value) {
    item = !!Data[0];
  } else {
    std::memcpy(&item, Data, N);
  }
  Data += N;
  Size -= N;

#define SEPARATE_ALLOCATION 0
#if SEPARATE_ALLOCATION
  // allocates as tight as possible, making it easier to catch buffer overruns.
  std::vector<char> fmtstringbuffer(Size);
  std::memcpy(fmtstringbuffer.data(), Data, Size);
  auto fmtstring = fmt::string_view(fmtstringbuffer.data(), Size);
#else
  auto fmtstring = fmt::string_view((const char*)Data, Size);
#endif
#define ALLOCATE_RESULT_IN_STRING 0
#if ALLOCATE_RESULT_IN_STRING
  std::string message = fmt::format(fmtstring, item);
#else
  fmt::memory_buffer message;
  fmt::format_to(message, fmtstring, item);
#endif
}

void invoke_fmt_time(const uint8_t* Data, std::size_t Size) {
  using Item = std::time_t;
  const auto N = sizeof(Item);
  if (Size <= N) {
    return;
  }
  Item item{};
  std::memcpy(&item, Data, N);
  Data += N;
  Size -= N;
#if SEPARATE_ALLOCATION
  // allocates as tight as possible, making it easier to catch buffer overruns.
  std::vector<char> fmtstringbuffer(Size);
  std::memcpy(fmtstringbuffer.data(), Data, Size);
  auto fmtstring = fmt::string_view(fmtstringbuffer.data(), Size);
#else
  auto fmtstring = fmt::string_view((const char*)Data, Size);
#endif
  auto* b = std::localtime(&item);
  if (b) {
#if ALLOCATE_RESULT_IN_STRING
    std::string message = fmt::format(fmtstring, *b);
#else
    fmt::memory_buffer message;
    fmt::format_to(message, fmtstring, *b);
#endif
  }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, std::size_t Size) {
  if (Size <= 3) {
    return 0;
  }

  const auto first = Data[0];
  Data++;
  Size--;

  try {
    switch (first) {
    case 0:
      invoke_fmt<bool>(Data, Size);
      break;
    case 1:
      invoke_fmt<char>(Data, Size);
      break;
    case 2:
      invoke_fmt<unsigned char>(Data, Size);
      break;
    case 3:
      invoke_fmt<signed char>(Data, Size);
      break;
    case 4:
      invoke_fmt<short>(Data, Size);
      break;
    case 5:
      invoke_fmt<unsigned short>(Data, Size);
      break;
    case 6:
      invoke_fmt<int>(Data, Size);
      break;
    case 7:
      invoke_fmt<unsigned int>(Data, Size);
      break;
    case 8:
      invoke_fmt<long>(Data, Size);
      break;
    case 9:
      invoke_fmt<unsigned long>(Data, Size);
      break;
    case 10:
      invoke_fmt<float>(Data, Size);
      break;
    case 11:
      invoke_fmt<double>(Data, Size);
      break;
    case 12:
      invoke_fmt<long double>(Data, Size);
      break;
    case 13:
      invoke_fmt_time(Data, Size);
      break;
    default:
      break;
    }
  } catch (std::exception& e) {
  }
  return 0;
}

#ifdef IMPLEMENT_MAIN
#  include <cassert>
#  include <fstream>
#  include <sstream>
#  include <vector>
int main(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) {
    std::ifstream in(argv[i]);
    assert(in);
    in.seekg(0, std::ios_base::end);
    const auto pos = in.tellg();
    in.seekg(0, std::ios_base::beg);
    std::vector<char> buf(pos);
    in.read(buf.data(), buf.size());
    assert(in.gcount() == pos);
    LLVMFuzzerTestOneInput((const uint8_t*)buf.data(), buf.size());
  }
}
#endif
