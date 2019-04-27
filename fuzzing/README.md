# FMT Fuzzer
This is for fuzzing libfmt which is proposed for standardization, so it's extra
important that bugs are smoked out.

It has found bugs:
- [fmt github #1124](https://github.com/fmtlib/fmt/issues/1124)
- [fmt github #1127](https://github.com/fmtlib/fmt/issues/1127)

Unfortunately one has to limit the maximum memory allocation, otherwise
the fuzzing will soon interrupt after trying to allocate many GB of memory.
Therefore, the code includes blocks like:
```cpp
#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
if(spec.precision>100000) {
 throw std::runtime_error("fuzz mode - avoiding large precision");
}
#endif
```
This macro is the defacto standard for making fuzzing practically possible, see [the libFuzzer documentation](https://llvm.org/docs/LibFuzzer.html#fuzzer-friendly-build-mode).

To do a variety of builds making sure the build system works as intended,
execute ```./build.sh```.

