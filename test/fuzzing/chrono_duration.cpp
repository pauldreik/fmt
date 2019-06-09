// Copyright (c) 2019, Paul Dreik
// License: see LICENSE.rst in the fmt root directory

#include <fmt/core.h>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <fmt/chrono.h>

template <typename Item, typename Ratio>
void invoke_inner(fmt::string_view formatstring, const Item item) {
  const std::chrono::duration<Item, Ratio> value(item);
  try {
      // Don't switch these two dynamically,
      // there is already a large combinatoric explosion
      // of type and ratio, causing afl to suffer and the corpus
      // getting enormous. Occasionally, flip this switch and
      // try manually.
#if 0
    std::string message = fmt::format(formatstring, value);
#else
    fmt::memory_buffer buf;
    fmt::format_to(buf, formatstring, value);
#endif
  } catch (std::exception& e) {
  }
}

// Item is the underlying type for duration (int, long etc)
template <typename Item>
void invoke_outer(const uint8_t* Data, std::size_t Size, const int scaling) {
    //always use a fixed location of the data, so different cases will
    //cooperate better. the same bit pattern, interpreted as another type,
    //is likely interesting.
    const auto Nfixed=std::max(sizeof(long double),sizeof(std::intmax_t));
  constexpr auto N = sizeof(Item);
  static_assert(N<=Nfixed, "fixed size is too small");
  if (Size <= Nfixed + 1) {
    return;
  }
  static_assert(std::is_trivially_copyable<Item>::value, "Item must be blittable");
  Item item{};
  std::memcpy(&item, Data, N);

  //fast forward
  Data += Nfixed;
  Size -= Nfixed;

  // Data is already allocated separately in libFuzzer so reading past
  // the end will most likely be detected anyway
  const auto formatstring=fmt::string_view((const char*)Data, Size);

  // doit_impl<Item,std::yocto>(buf.data(),item);
  // doit_impl<Item,std::zepto>(buf.data(),item);
  switch (scaling) {
  case 1:
      invoke_inner<Item, std::atto>(formatstring, item);
      break;
  case 2:
      invoke_inner<Item, std::femto>(formatstring, item);
      break;
  case 3:
      invoke_inner<Item, std::pico>(formatstring, item);
      break;
  case 4:
      invoke_inner<Item, std::nano>(formatstring, item);
      break;
  case 5:
      invoke_inner<Item, std::micro>(formatstring, item);
      break;
  case 6:
      invoke_inner<Item, std::milli>(formatstring, item);
      break;
  case 7:
      invoke_inner<Item, std::centi>(formatstring, item);
      break;
  case 8:
      invoke_inner<Item, std::deci>(formatstring, item);
      break;
  case 9:
      invoke_inner<Item, std::deca>(formatstring, item);
      break;
  case 10:
      invoke_inner<Item, std::kilo>(formatstring, item);
      break;
  case 11:
      invoke_inner<Item, std::mega>(formatstring, item);
      break;
  case 12:
      invoke_inner<Item, std::giga>(formatstring, item);
     break;
  case 13:
      invoke_inner<Item, std::tera>(formatstring, item);
      break;
  case 14:
      invoke_inner<Item, std::peta>(formatstring, item);
      break;
  case 15:
      invoke_inner<Item, std::exa>(formatstring, item);
  }
  // doit_impl<Item,std::zeta>(buf.data(),item);
  // doit_impl<Item,std::yotta>(buf.data(),item);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, std::size_t Size) {
  if (Size <= 4) {
    return 0;
  }

  const auto representation = Data[0];
  const auto scaling = Data[1];
  Data+=2;
  Size-=2;

  switch (representation) {
  case 1:
    invoke_outer<char>(Data, Size, scaling);
    break;
  case 21:
    invoke_outer<unsigned char>(Data, Size, scaling);
    break;
  case 31:
    invoke_outer<signed char>(Data, Size, scaling);
    break;
  case 2:
    invoke_outer<short>(Data, Size, scaling);
    break;
  case 22:
    invoke_outer<unsigned short>(Data, Size, scaling);
    break;
  case 3:
    invoke_outer<int>(Data, Size, scaling);
    break;
  case 23:
    invoke_outer<unsigned int>(Data, Size, scaling);
    break;
  case 4:
    invoke_outer<long>(Data, Size, scaling);
    break;
  case 24:
    invoke_outer<unsigned long>(Data, Size, scaling);
    break;
  case 5:
    invoke_outer<float>(Data, Size, scaling);
    break;
  case 6:
    invoke_outer<double>(Data, Size, scaling);
    break;
  case 7:
    invoke_outer<long double>(Data, Size, scaling);
    break;
  default:
    break;
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
