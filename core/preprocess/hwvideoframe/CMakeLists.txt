cmake_minimum_required(VERSION 3.2)
project(hw-frame-extract)
# SET(CUDA_VERSION 10.1)

#gcc version
#GCC('gcc482')
#CUDA("10.1")
set(global_cflags_str "-g -pipe -W -Wall -fPIC")
set(CMAKE_C_FLAGS ${global_cflags_str})

#C++ flags.
set(global_cxxflags_str "-g -pipe -W -Wall -fPIC -std=c++11")
set(CMAKE_CXX_FLAGS ${global_cxxflags_str})

add_subdirectory(cuda)
add_subdirectory(pybind11)
set (EXTRA_LIBS ${EXTRA_LIBS} gpu)

message(${CMAKE_CURRENT_SOURCE_DIR})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/pybind11/include")
include_directories("/opt/compiler/cuda-10.1/include")
include_directories("/home/work/wubohao/baidu/third-party/python/include/python2.7")

file(GLOB SOURCE_FILES src/*.cpp pybind11/*.cpp )

link_directories("-L/opt/compiler/cuda-10.1/lib64 -lcudart -lnppidei_static -lnppial_static -lnpps_static -lnppc_static -lculibos")
# link_directories("/home/work/wubohao/baidu/third-party/python/lib")

#.so
add_library(gpupreprocess SHARED ${SOURCE_FILES})
target_link_libraries (gpupreprocess ${EXTRA_LIBS})
