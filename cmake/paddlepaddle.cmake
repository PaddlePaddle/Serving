# Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
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
# limitations under the License.

INCLUDE(ExternalProject)

SET(PADDLE_SOURCES_DIR ${THIRD_PARTY_PATH}/Paddle)
SET(PADDLE_DOWNLOAD_DIR ${PADDLE_SOURCES_DIR}/src/extern_paddle)
SET(PADDLE_INSTALL_DIR ${THIRD_PARTY_PATH}/install/Paddle/)
SET(PADDLE_INCLUDE_DIR "${PADDLE_INSTALL_DIR}/include" CACHE PATH "PaddlePaddle include directory." FORCE)
SET(PADDLE_LIBRARIES "${PADDLE_INSTALL_DIR}/lib/libpaddle_fluid.a" CACHE FILEPATH "Paddle library." FORCE)


# Reference https://stackoverflow.com/questions/45414507/pass-a-list-of-prefix-paths-to-externalproject-add-in-cmake-args
set(prefix_path "${THIRD_PARTY_PATH}/install/gflags|${THIRD_PARTY_PATH}/install/leveldb|${THIRD_PARTY_PATH}/install/snappy|${THIRD_PARTY_PATH}/install/gtest|${THIRD_PARTY_PATH}/install/protobuf|${THIRD_PARTY_PATH}/install/zlib|${THIRD_PARTY_PATH}/install/glog")

message( "WITH_GPU = ${WITH_GPU}")


# Paddle Version should be one of:
# latest: latest develop build
# version number like 1.5.2
SET(PADDLE_VERSION "latest")

if (WITH_GPU)
    SET(PADDLE_LIB_VERSION "${PADDLE_VERSION}-gpu-cuda${CUDA_VERSION_MAJOR}-cudnn7-avx-mkl")
else()
    if (AVX_FOUND)
        if (WITH_MKLML)
            SET(PADDLE_LIB_VERSION "${PADDLE_VERSION}-cpu-avx-mkl")
        else()
            SET(PADDLE_LIB_VERSION "${PADDLE_VERSION}-cpu-avx-openblas")
        endif()
    else()
        SET(PADDLE_LIB_VERSION "${PADDLE_VERSION}-cpu-noavx-openblas")
    endif()
endif()

SET(PADDLE_LIB_PATH "http://paddle-inference-lib.bj.bcebos.com/${PADDLE_LIB_VERSION}/fluid_inference.tgz")
MESSAGE(STATUS "PADDLE_LIB_PATH=${PADDLE_LIB_PATH}")

ExternalProject_Add(
    "extern_paddle"
    ${EXTERNAL_PROJECT_LOG_ARGS}
    URL                 "${PADDLE_LIB_PATH}"
    PREFIX              "${PADDLE_SOURCES_DIR}"
    DOWNLOAD_DIR        "${PADDLE_DOWNLOAD_DIR}"
    CONFIGURE_COMMAND   ""
    BUILD_COMMAND       ""
    UPDATE_COMMAND      ""
    INSTALL_COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${PADDLE_DOWNLOAD_DIR}/paddle/include ${PADDLE_INSTALL_DIR}/include &&
        ${CMAKE_COMMAND} -E copy_directory ${PADDLE_DOWNLOAD_DIR}/paddle/lib ${PADDLE_INSTALL_DIR}/lib &&
        ${CMAKE_COMMAND} -E copy_directory ${PADDLE_DOWNLOAD_DIR}/third_party ${PADDLE_INSTALL_DIR}/third_party
)

INCLUDE_DIRECTORIES(${PADDLE_INCLUDE_DIR})

ADD_LIBRARY(paddle_fluid STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET paddle_fluid PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/lib/libpaddle_fluid.a)

LIST(APPEND external_project_dependencies paddle)

#ADD_LIBRARY(snappystream STATIC IMPORTED GLOBAL)
#SET_PROPERTY(TARGET snappystream PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/third_party/install/snappystream/lib/libsnappystream.a)

ADD_LIBRARY(xxhash STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET xxhash PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/third_party/install/xxhash/lib/libxxhash.a)

ADD_LIBRARY(iomp5 SHARED IMPORTED GLOBAL)
SET_PROPERTY(TARGET iomp5 PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/third_party/install/mklml/lib/libiomp5.so)

ADD_LIBRARY(mklml_intel SHARED IMPORTED GLOBAL)
SET_PROPERTY(TARGET mklml_intel PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/third_party/install/mklml/lib/libmklml_intel.so)

ADD_LIBRARY(mkldnn SHARED IMPORTED GLOBAL)
SET_PROPERTY(TARGET mkldnn PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/third_party/install/mkldnn/lib/libmkldnn.so.0)

LIST(APPEND paddle_depend_libs
        #        snappystream
        snappy
        iomp5
        mklml_intel
        mkldnn
        xxhash
        )
