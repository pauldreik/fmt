# for fuzzing libfmt http://fmtlib.net/
#
# by Paul Dreik 20190420
# https://www.pauldreik.se/


#cmake_minimum_required(VERSION 3.10)

#project(fmt_fuzzers LANGUAGES CXX)

#add_definitions(-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION=1)

#add_subdirectory(fmt)


# settings this links in a main. useful for reproducing,
# kcov, gdb, afl, valgrind.
# (note that libFuzzer can also reproduce, just pass it the files)
option(FMT_FUZZ_LINKMAIN "enables the reproduce mode, instead of libFuzzer" On)

#for oss-fuzz - insert $LIB_FUZZING_ENGINE into the link flags, but only for
#the fuzz targets, otherwise the cmake configuration step fails.
option(FMT_FUZZ_LDFLAGS "LDFLAGS for the fuzz targets" "")

#find all fuzzers.
file(GLOB SOURCES "*.cpp")

if(FMT_FUZZ_LINKMAIN)
  add_definitions(-DIMPLEMENT_MAIN=1)
endif()



macro(implement_fuzzer sourcefile)
  get_filename_component(basename ${sourcefile} NAME_WE)
  set(name fuzzer_${basename})
  add_executable(${name} ${sourcefile})
  target_link_libraries(${name}  PRIVATE fmt)
if(FMT_FUZZ_LDFLAGS)
  target_link_libraries(${name} PRIVATE ${FMT_FUZZ_LDFLAGS})
endif()
  set_property(TARGET ${name} PROPERTY CXX_STANDARD 17)
endmacro()

foreach(X IN ITEMS ${SOURCES})
    implement_fuzzer(${X})
endforeach()
