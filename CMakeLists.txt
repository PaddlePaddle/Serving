# Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License

cmake_minimum_required(VERSION 3.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(PADDLE_SERVING_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PADDLE_SERVING_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
SET(PADDLE_SERVING_INSTALL_DIR ${CMAKE_BINARY_DIR}/output)
SET(CMAKE_INSTALL_RPATH "\$ORIGIN" "${CMAKE_INSTALL_RPATH}")

include(system)

project(paddle-serving CXX C)
message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER}, version: "
        "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}, version: "
        "${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")


find_package(Git REQUIRED)
find_package(Threads REQUIRED)
find_package(CUDA QUIET)

include(simd)

# CMAKE_BUILD_TYPE
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel"
      FORCE)
endif()

set(THIRD_PARTY_PATH "${CMAKE_BINARY_DIR}/third_party" CACHE STRING
  "A path setting third party libraries download & build directories.")

set(THIRD_PARTY_BUILD_TYPE Release)

option(WITH_AVX	    "Compile Paddle Serving with AVX intrinsics"    OFF)
option(WITH_MKL	    "Compile Paddle Serving with MKL support."      OFF)
option(WITH_GPU	    "Compile Paddle Serving with NVIDIA GPU"        OFF)
option(CLIENT  	    "Compile Paddle Serving Client"		    OFF)
option(SERVER	    "Compile Paddle Serving Server"		    OFF)
option(APP          "Compile Paddle Serving App package"	    OFF)
option(WITH_ELASTIC_CTR "Compile ELASITC-CTR solution"              OFF)
option(PACK         "Compile for whl"                               OFF)
option(WITH_TRT     "Compile Paddle Serving with TRT"       OFF)

set(WITH_MKLML ${WITH_MKL})
if (NOT DEFINED WITH_MKLDNN)
    if (WITH_MKL AND AVX2_FOUND)
        set(WITH_MKLDNN ON)
    else()
        message(STATUS "Do not have AVX2 intrinsics and disabled MKL-DNN")
        set(WITH_MKLDNN OFF)
    endif()
endif()

if (SERVER)
include(external/jsoncpp)
#include(external/rocksdb)
endif()

if (SERVER OR CLIENT)
include(external/snappy)
include(external/leveldb)
include(external/zlib)
include(external/boost)
include(external/protobuf)
include(external/brpc)
include(external/gflags)
include(external/glog)
include(external/pybind11)
include(external/python)
include(generic)
include(flags)
endif()

if (APP)
include(external/zlib)
include(external/boost)
include(external/protobuf)
include(external/gflags)
include(external/glog)
include(external/pybind11)
include(external/python)
include(generic)
endif()

if (SERVER)
include(external/cudnn)
include(paddlepaddle)
endif()

message("paddle serving source dir: " ${PADDLE_SERVING_SOURCE_DIR})
include_directories(${PADDLE_SERVING_SOURCE_DIR})
include_directories(${PADDLE_SERVING_BINARY_DIR})

if(SERVER)
    set(EXTERNAL_LIBS
	jsoncpp
	gflags
	rocksdb
	glog
	protobuf
	paddlepaddle
	brpc)
endif()

set(EXTERNAL_LIBS
	gflags
	glog
	protobuf
	brpc
)

if(SERVER)
if(WITH_MKLML)
    list(APPEND EXTERNAL_LIBS ${MKLML_IOMP_LIB})
endif()
endif()


if(SERVER)
if(WITH_MKLDNN)
    list(APPEND EXTERNAL_LIBS ${MKLDNN_LIB})
endif()
endif()

if (SERVER)
    list(APPEND EXTERNAL_LIBS paddlepaddle)
endif()

add_subdirectory(core)

if(SERVER)
add_subdirectory(paddle_inference)
endif()

add_subdirectory(python)
