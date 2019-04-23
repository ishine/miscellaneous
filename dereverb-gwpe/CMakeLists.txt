cmake_minimum_required (VERSION 2.8)
project(gwpe)

set(CMAKE_F "-Wall -std=c++11 -fPIC -O3 -DHAVE_CLAPACK -msse -msse2 -pthread -framework Accelerate -lm -lpthread -ldl")
set(CMAKE_CXX_FLAGS ${CMAKE_F})
set(CMAKE_MACOSX_RPATH 0)

include_directories(${CMAKE_CURRENT_LIST_DIR}/ ${CMAKE_CURRENT_LIST_DIR}/dereverb ${CMAKE_CURRENT_LIST_DIR}/../)
include_directories("/Users/danhui/kaldi/src/" "/Users/danhui/kaldi/tools/openfst/include")

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/dereverb base-src)
add_library(gwpe ${base-src})

link_directories("/Users/danhui/kaldi/src/lib/")
add_executable(apply-gwpe apply-gwpe.cc)
target_link_libraries(apply-gwpe gwpe kaldi-base.a kaldi-util.a kaldi-matrix.a kaldi-feat.a)
