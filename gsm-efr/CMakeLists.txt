cmake_minimum_required (VERSION 2.8)
project(gsm-efr-simulator)

set(CMAKE_F "-Wall -std=c++11 -fPIC -O3")
# -DHAVE_CLAPACK -msse -msse2 -pthread -framework Accelerate -lm -lpthread -ldl")
set(CMAKE_CXX_FLAGS ${CMAKE_F})
set(CMAKE_MACOSX_RPATH 1)

include_directories(${CMAKE_CURRENT_LIST_DIR}/c-code)
include_directories("/Users/danhui/kaldi/src/" "/Users/danhui/kaldi/tools/openfst/include")
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/c-code gsm-src)
add_library(gsm-efr gsm-efr-wrapper.cc ${gsm-src})

link_directories("/Users/danhui/kaldi/src/lib/")
add_executable(simulate-gsm-efr simulate-gsm-efr.cc)
target_link_libraries(simulate-gsm-efr gsm-efr kaldi-base.a kaldi-util.a kaldi-matrix.a kaldi-feat.a)


