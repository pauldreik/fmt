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
void doit_impl(const char* formatstring, const Item item) {
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

  // allocates as tight as possible, making it easier to catch buffer overruns
  // also, make it null terminated.
  std::vector<char> buf(Size + 1);
  std::memcpy(buf.data(), Data, Size);
  // doit_impl<Item,std::yocto>(buf.data(),item);
  // doit_impl<Item,std::zepto>(buf.data(),item);
  switch (scaling) {
  case 1:
      doit_impl<Item, std::atto>(buf.data(), item);
      break;
  case 2:
      doit_impl<Item, std::femto>(buf.data(), item);
      break;
  case 3:
      doit_impl<Item, std::pico>(buf.data(), item);
      break;
  case 4:
      doit_impl<Item, std::nano>(buf.data(), item);
      break;
  case 5:
      doit_impl<Item, std::micro>(buf.data(), item);
      break;
  case 6:
      doit_impl<Item, std::milli>(buf.data(), item);
      break;
  case 7:
      doit_impl<Item, std::centi>(buf.data(), item);
      break;
  case 8:
      doit_impl<Item, std::deci>(buf.data(), item);
      break;
  case 9:
      doit_impl<Item, std::deca>(buf.data(), item);
      break;
  case 10:
      doit_impl<Item, std::kilo>(buf.data(), item);
      break;
  case 11:
      doit_impl<Item, std::mega>(buf.data(), item);
      break;
  case 12:
      doit_impl<Item, std::giga>(buf.data(), item);
     break;
  case 13:
      doit_impl<Item, std::tera>(buf.data(), item);
      break;
  case 14:
      doit_impl<Item, std::peta>(buf.data(), item);
      break;
  case 15:
      doit_impl<Item, std::exa>(buf.data(), item);
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
