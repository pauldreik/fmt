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
void doit_impl(fmt::string_view formatstring, const Item item) {
  const std::chrono::duration<Item, Ratio> value(item);
  try {
    std::string message = fmt::format(formatstring, value);
  } catch (std::exception& e) {
  }
}

// Item is the underlying type for duration (int, long etc)
template <typename Item>
void doit(const uint8_t* Data, std::size_t Size, const int scaling) {
    //always use a fixed location of the data, so different cases will
    //cooperate better. the same bit pattern, interpreted as another type,
    //is likely interesting.
    const auto Nfixed=std::max(sizeof(long double),sizeof(std::intmax_t));
  const auto N = sizeof(Item);
  static_assert(N<=Nfixed,"fixed size is too small");
  if (Size <= Nfixed + 1) {
    return;
  }
  static_assert(std::is_trivially_constructible<Item>::value,"Item must be blittable");
  Item item{};
  std::memcpy(&item, Data, N);

  //fast forward
  Data += Nfixed;
  Size -= Nfixed;

  // see https://github.com/fmtlib/fmt/issues/1178
  const bool github_1178_is_solved = true;
  if (!github_1178_is_solved) {
    if (std::is_floating_point<Item>::value ||
        std::numeric_limits<Item>::is_signed) {
      if (item < 0) {
        return;
      }
    }
  }

  // Data is already allocated separately in libFuzzer so reading past
  // the end will most likely be detected anyway

  // see https://github.com/fmtlib/fmt/issues/1194
#define GITHUB_1194_IS_SOLVED 0
#if GITHUB_1194_IS_SOLVED
  const auto formatstring=fmt::string_view((const char*)Data, Size);
#else
  // needs a null terminator, so allocate separately
  std::vector<char> buf(Size+1);
  std::memcpy(buf.data(), Data, Size);
  const auto formatstring=fmt::string_view(buf.data(), Size);
#endif



  // doit_impl<Item,std::yocto>(buf.data(),item);
  // doit_impl<Item,std::zepto>(buf.data(),item);
  switch (scaling) {
  case 1:
      doit_impl<Item, std::atto>(formatstring, item);
      break;
  case 2:
      doit_impl<Item, std::femto>(formatstring, item);
      break;
  case 3:
      doit_impl<Item, std::pico>(formatstring, item);
      break;
  case 4:
      doit_impl<Item, std::nano>(formatstring, item);
      break;
  case 5:
      doit_impl<Item, std::micro>(formatstring, item);
      break;
  case 6:
      doit_impl<Item, std::milli>(formatstring, item);
      break;
  case 7:
      doit_impl<Item, std::centi>(formatstring, item);
      break;
  case 8:
      doit_impl<Item, std::deci>(formatstring, item);
      break;
  case 9:
      doit_impl<Item, std::deca>(formatstring, item);
      break;
  case 10:
      doit_impl<Item, std::kilo>(formatstring, item);
      break;
  case 11:
      doit_impl<Item, std::mega>(formatstring, item);
      break;
  case 12:
      doit_impl<Item, std::giga>(formatstring, item);
     break;
  case 13:
      doit_impl<Item, std::tera>(formatstring, item);
      break;
  case 14:
      doit_impl<Item, std::peta>(formatstring, item);
      break;
  case 15:
      doit_impl<Item, std::exa>(formatstring, item);
  }
  // doit_impl<Item,std::zeta>(buf.data(),item);
  // doit_impl<Item,std::yotta>(buf.data(),item);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, std::size_t Size) {
  if (Size <= 4) {
    return 0;
  }

  const auto first = Data[0];
  Data++;
  Size--;
  const auto second = Data[0];
  Data++;
  Size--;

  switch (first) {
  case 1:
    doit<char>(Data, Size, second);
    break;
  case 21:
    doit<unsigned char>(Data, Size, second);
    break;
  case 31:
    doit<signed char>(Data, Size, second);
    break;
  case 2:
    doit<short>(Data, Size, second);
    break;
  case 22:
    doit<unsigned short>(Data, Size, second);
    break;
  case 3:
    doit<int>(Data, Size, second);
    break;
  case 23:
    doit<unsigned int>(Data, Size, second);
    break;
  case 4:
    doit<long>(Data, Size, second);
    break;
  case 24:
    doit<unsigned long>(Data, Size, second);
    break;
  case 5:
    doit<float>(Data, Size, second);
    break;
  case 6:
    doit<double>(Data, Size, second);
    break;
  case 7:
    doit<long double>(Data, Size, second);
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
