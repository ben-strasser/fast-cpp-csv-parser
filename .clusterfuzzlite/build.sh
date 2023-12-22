#!/bin/bash -eu
# Supply build instructions
# Use the following environment variables to build the code
# $CXX:               c++ compiler
# $CC:                c compiler
# CFLAGS:             compiler flags for C files
# CXXFLAGS:           compiler flags for CPP files
# LIB_FUZZING_ENGINE: linker flag for fuzzing harnesses

# Copy all fuzzer executables to $OUT/

# Copy all fuzzer executables to $OUT/
$CXX $CFLAGS $LIB_FUZZING_ENGINE \
  $SRC/fast-cpp-csv-parser/.clusterfuzzlite/parse_fuzzer.cpp \
  -o $OUT/parse_fuzzer \
  -I$SRC/fast-cpp-csv-parser
