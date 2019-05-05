#!/bin/sh
#
# Creates fuzzer builds of various kinds
# - reproduce mode (no fuzzing, just enables replaying data through the fuzzers)
# - oss-fuzz emulated mode (makes sure a simulated invocation by oss-fuzz works)
# - libFuzzer build (you will need clang)
# - afl build (you will need afl)
#
#
# Copyright (c) 2019 Paul Dreik
#
# License: see LICENSE.rst in the fmt root directory

set -e
me=$(basename $0)
root=$(readlink -f "$(dirname "$0")/..")


echo $me: root=$root

here=$(pwd)

#builds the fuzzers as one would do if using afl or just making
#binaries for reproducing.
builddir=$here/build-fuzzers-reproduce
mkdir -p $builddir
cd $builddir
CXX="ccache g++" CXXFLAGS="-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION=1" cmake \
$root -GNinja -DCMAKE_BUILD_TYPE=Debug -DFMT_DOC=Off -DFMT_TEST=Off -DFMT_FUZZ=On
cmake --build $builddir



#builds the fuzzers as oss-fuzz does
builddir=$here/build-fuzzers-ossfuzz
mkdir -p $builddir
cd $builddir
CXX="clang++" \
CXXFLAGS="-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION=1 -fsanitize=fuzzer-no-link" \
cmake $root -GNinja -DCMAKE_BUILD_TYPE=Debug \
-DFMT_DOC=Off \
-DFMT_TEST=Off \
-DFMT_FUZZ=On \
-DFMT_FUZZ_LINKMAIN=Off \
-DFMT_FUZZ_LDFLAGS="-fsanitize=fuzzer"

cmake --build $builddir


#builds fuzzers for local fuzzing with libfuzzer with asan+usan
builddir=$here/build-fuzzers-libfuzzer
mkdir -p $builddir
cd $builddir
CXX="clang++" \
CXXFLAGS="-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION=1 -fsanitize=fuzzer-no-link,address,undefined" \
cmake $root -GNinja -DCMAKE_BUILD_TYPE=Debug \
-DFMT_DOC=Off \
-DFMT_TEST=Off \
-DFMT_FUZZ=On \
-DFMT_FUZZ_LINKMAIN=Off \
-DFMT_FUZZ_LDFLAGS="-fsanitize=fuzzer"

cmake --build $builddir

#builds fuzzers for local fuzzing with libfuzzer with asan only
builddir=$here/build-fuzzers-libfuzzer-addr
mkdir -p $builddir
cd $builddir
CXX="clang++" \
CXXFLAGS="-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION=1 -fsanitize=fuzzer-no-link,address" \
cmake $root -GNinja -DCMAKE_BUILD_TYPE=Debug \
-DFMT_DOC=Off \
-DFMT_TEST=Off \
-DFMT_FUZZ=On \
-DFMT_FUZZ_LINKMAIN=Off \
-DFMT_FUZZ_LDFLAGS="-fsanitize=fuzzer"

cmake --build $builddir




#builds fuzzers for local fuzzing with afl
builddir=$here/build-fuzzers-afl
mkdir -p $builddir
cd $builddir
CXX="afl-g++" \
CXXFLAGS="-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION=1 -fsanitize=address,undefined" \
cmake $root -GNinja -DCMAKE_BUILD_TYPE=Debug \
-DFMT_DOC=Off \
-DFMT_TEST=Off \
-DFMT_FUZZ=On \
-DFMT_FUZZ_LINKMAIN=On

cmake --build $builddir


echo $me: all good

