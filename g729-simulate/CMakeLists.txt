cmake_minimum_required (VERSION 2.8)
project(g729fp)

set(CMAKE_F "-Wall -std=c++11 -fPIC -O3 -DHAVE_CLAPACK -msse -msse2 -pthread -framework Accelerate -lm -lpthread -ldl")
set(CMAKE_CXX_FLAGS ${CMAKE_F})
set(CMAKE_MACOSX_RPATH 0)

include_directories(${CMAKE_CURRENT_LIST_DIR}/src ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories("/Users/danhui/kaldi/src/" "/Users/danhui/kaldi/tools/openfst/include")

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src base-src)
add_library(g729fp ${base-src})

add_executable(encoder ${CMAKE_CURRENT_LIST_DIR}/src/encoderTest.c)
target_link_libraries(encoder g729fp)

add_executable(decoder ${CMAKE_CURRENT_LIST_DIR}/src/decoderTest.c)
target_link_libraries(decoder g729fp)

link_directories("/Users/danhui/kaldi/src/lib/")
add_executable(simulate-g729 simulate-g729.cc)
target_link_libraries(simulate-g729 g729fp kaldi-base.a kaldi-util.a kaldi-matrix.a kaldi-feat.a)
