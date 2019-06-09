// Copyright (c) 2019, Paul Dreik
// License: see LICENSE.rst in the fmt root directory

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

template <typename Item1>
void invoke_fmt(const uint8_t* Data, std::size_t Size, int argsize) {
  const auto N1 = sizeof(Item1);
  if (Size <= N1) {
    return;
  }
  Item1 item1{};
  if /*constexpr*/ (std::is_same<Item1, bool>::value) {
    item1 = !!Data[0];
  } else {
    std::memcpy(&item1, Data, N1);
  }
  Data += N1;
  Size -= N1;

  // how many chars should be used for the argument name?
  if (argsize <= 0 || argsize >= Size) {
    return;
  }

  //allocating buffers separately is slower, but increases chances
  //of detecting memory errors
#define SEPARATE_ALLOCATION 1
#if SEPARATE_ALLOCATION
  std::vector<char> argnamebuffer(argsize);
  std::memcpy(argnamebuffer.data(), Data, argsize);
  auto argname=fmt::string_view(argnamebuffer.data(),argsize);
        #else
   auto argname=fmt::string_view((const char*)Data,argsize);
#endif
  Data += argsize;
  Size -= argsize;

#if SEPARATE_ALLOCATION
  // allocates as tight as possible, making it easier to catch buffer overruns.
  std::vector<char> fmtstringbuffer(Size);
  std::memcpy(fmtstringbuffer.data(), Data, Size);
  auto fmtstring=fmt::string_view(fmtstringbuffer.data(),Size);
#else
  auto fmtstring=fmt::string_view((const char*)Data,Size);
#endif
  std::string message =
      fmt::format(fmtstring, fmt::arg(argname, item1));
#undef SEPARATE_ALLOCATION
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
  case 11:
      using sc=signed char;
    callback(sc{});
    break;
  case 21:
      using uc=unsigned char;
    callback(uc{});
    break;
  case 2:
    callback(short{});
    break;
  case 22:
      using us=unsigned short;
    callback(us{});
    break;
  case 3:
    callback(int{});
    break;
  case 13:
    callback(unsigned{});
    break;
  case 4:
    callback(long{});
    break;
  case 14:
      using ul=unsigned long;
    callback(ul{});
    break;
  case 5:
    callback(float{});
    break;
  case 6:
    callback(double{});
    break;
  case 7:
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

  auto outerfcn = [=](auto param1) { invoke_fmt<decltype(param1)>(Data, Size, second); };

  try {
    invoke(first, outerfcn);
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
