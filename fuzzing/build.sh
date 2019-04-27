#!/bin/sh
#
#
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


#builds fuzzers for local fuzzing with libfuzzer
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


echo $me: all good

