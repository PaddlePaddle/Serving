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
# SET(CMAKE_BUILD_TYPE "Debug")
# CMAKE_BUILD_TYPE
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel"
      FORCE)
endif()
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set(THIRD_PARTY_PATH "${CMAKE_BINARY_DIR}/third_party" CACHE STRING
  "A path setting third party libraries download & build directories.")

set(THIRD_PARTY_BUILD_TYPE Release)

option(WITH_AVX	            "Compile Paddle Serving with AVX intrinsics"        OFF)
option(WITH_MKL	            "Compile Paddle Serving with MKL support."          OFF)
option(WITH_GPU	            "Compile Paddle Serving with NVIDIA GPU"            OFF)
option(WITH_LITE            "Compile Paddle Serving with Paddle Lite Engine"    OFF)
option(WITH_XPU	            "Compile Paddle Serving with Baidu Kunlun"          OFF)
option(WITH_PYTHON          "Compile Paddle Serving with Python"                 ON)
option(CLIENT  	            "Compile Paddle Serving Client"		                OFF)
option(SERVER	            "Compile Paddle Serving Server"		                OFF)
option(APP                  "Compile Paddle Serving App package"	            OFF)
option(WITH_ELASTIC_CTR     "Compile ELASITC-CTR solution"                      OFF)
option(PACK                 "Compile for whl"                                   OFF)
option(WITH_TRT             "Compile Paddle Serving with TRT"                   OFF)
option(PADDLE_ON_INFERENCE  "Compile for encryption"                             ON)
option(WITH_OPENCV	    "Compile Paddle Serving with OPENCV"                    OFF)
option(WITH_ROCM	    "Compile Paddle Serving with ROCM"                    OFF)
option(WITH_ASCEND_CL       "Compile PaddlePaddle with ASCEND CL"               OFF)
option(WITH_JETSON       "Compile PaddlePaddle with JETSON"               OFF)

if(NOT DEFINED VERSION_TAG)
  set(VERSION_TAG "0.0.0")
endif()
if (WITH_PYTHON)
  message(STATUS "Compile Version Tag for wheel: ${VERSION_TAG}")
endif()

if (WITH_OPENCV)
    SET(OPENCV_DIR "" CACHE PATH "Location of libraries")
    if(NOT DEFINED OPENCV_DIR)
        message(FATAL_ERROR "please set OPENCV_DIR with -DOPENCV_DIR=/path/opencv")
    endif()
    if (WIN32)
    find_package(OpenCV REQUIRED PATHS ${OPENCV_DIR}/build/ NO_DEFAULT_PATH)
    else ()
    find_package(OpenCV REQUIRED PATHS ${OPENCV_DIR}/share/OpenCV NO_DEFAULT_PATH)
    endif ()
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()

if (PADDLE_ON_INFERENCE)
    add_definitions(-DPADDLE_ON_INFERENCE)
    message(STATUS "Use PADDLE_ON_INFERENCE")
endif()
set(WITH_MKLML ${WITH_MKL})
if (NOT DEFINED WITH_MKLDNN)
    if (WITH_MKL AND AVX2_FOUND)
        set(WITH_MKLDNN ON)
    else()
        message(STATUS "Do not have AVX2 intrinsics and disabled MKL-DNN")
        set(WITH_MKLDNN OFF)
    endif()
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
    include(external/utf8proc)
    if (WITH_PYTHON)
        include(external/pybind11)
        include(external/python)
    endif()
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
    include(external/jsoncpp)
    #include(external/rocksdb)
    include(external/cudnn)
    include(paddlepaddle)
    include(external/prometheus)
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

    if(WITH_MKLDNN)
        list(APPEND EXTERNAL_LIBS ${MKLDNN_LIB})
    endif()

    list(APPEND EXTERNAL_LIBS paddlepaddle)
    if(WITH_ROCM)
        include(hip)
    endif()

    if(WITH_ASCEND_CL)
	    include(external/ascend)
	    list(APPEND EXTERNAL_LIBS ascend ascend_cl)
    endif()
endif()


add_subdirectory(core)

if(SERVER)
    add_subdirectory(paddle_inference)
endif()

if (WITH_PYTHON)
    add_subdirectory(python)
endif()
