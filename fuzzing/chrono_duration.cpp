// Copyright (c) 2019, Paul Dreik
// License: see LICENSE.rst in the fmt root directory

#include <fmt/core.h>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <limits>

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
template <typename Item> void doit(const uint8_t* Data, std::size_t Size) {
  const auto N = sizeof(Item);
  if (Size <= N) {
    return;
  }
  Item item{};
  std::memcpy(&item, Data, N);
  Data += N;
  Size -= N;

  if(std::is_floating_point<Item>::value || std::numeric_limits<Item>::is_signed) {
      if(item<0) {
      //avoid problems with negative numbers until https://github.com/fmtlib/fmt/issues/1178 is solved
      return;
      }
  }

  // allocates as tight as possible, making it easier to catch buffer overruns
  // also, make it null terminated.
  std::vector<char> buf(Size + 1);
  std::memcpy(buf.data(), Data, Size);
  // doit_impl<Item,std::yocto>(buf.data(),item);
  // doit_impl<Item,std::zepto>(buf.data(),item);
  doit_impl<Item, std::atto>(buf.data(), item);
  doit_impl<Item, std::femto>(buf.data(), item);
  doit_impl<Item, std::pico>(buf.data(), item);
  doit_impl<Item, std::nano>(buf.data(), item);
  doit_impl<Item, std::micro>(buf.data(), item);
  doit_impl<Item, std::milli>(buf.data(), item);
  doit_impl<Item, std::centi>(buf.data(), item);
  doit_impl<Item, std::deci>(buf.data(), item);
  doit_impl<Item, std::deca>(buf.data(), item);
  doit_impl<Item, std::kilo>(buf.data(), item);
  doit_impl<Item, std::mega>(buf.data(), item);
  doit_impl<Item, std::giga>(buf.data(), item);
  doit_impl<Item, std::tera>(buf.data(), item);
  doit_impl<Item, std::peta>(buf.data(), item);
  doit_impl<Item, std::exa>(buf.data(), item);
  // doit_impl<Item,std::zeta>(buf.data(),item);
  // doit_impl<Item,std::yotta>(buf.data(),item);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, std::size_t Size) {
  if (Size <= 3) {
    return 0;
  }

  const auto first = Data[0];
  Data++;
  Size--;

    switch (first) {
    case 1:
      doit<char>(Data, Size);
      break;
    case 2:
      doit<short>(Data, Size);
      break;
    case 3:
      doit<int>(Data, Size);
      break;
    case 4:
      doit<long>(Data, Size);
      break;
    case 5:
      doit<float>(Data, Size);
      break;
    case 6:
      doit<double>(Data, Size);
      break;
    case 7:
      doit<long double>(Data, Size);
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
