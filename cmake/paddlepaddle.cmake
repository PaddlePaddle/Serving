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
SET(PADDLE_LIBRARIES "${PADDLE_INSTALL_DIR}/lib/libpaddle_inference.a" CACHE FILEPATH "Paddle library." FORCE)

message("paddle install dir: " ${PADDLE_INSTALL_DIR})

# Reference https://stackoverflow.com/questions/45414507/pass-a-list-of-prefix-paths-to-externalproject-add-in-cmake-args
set(prefix_path "${THIRD_PARTY_PATH}/install/gflags|${THIRD_PARTY_PATH}/install/leveldb|${THIRD_PARTY_PATH}/install/snappy|${THIRD_PARTY_PATH}/install/gtest|${THIRD_PARTY_PATH}/install/protobuf|${THIRD_PARTY_PATH}/install/zlib|${THIRD_PARTY_PATH}/install/glog")

message( "WITH_GPU = ${WITH_GPU}")


# Paddle Version should be one of:
# latest: latest develop build
# version number like 1.5.2
SET(PADDLE_VERSION "2.1.0")
if (WITH_GPU)
    if(CUDA_VERSION EQUAL 11.0)
        set(CUDA_SUFFIX "cuda11.0-cudnn8-mkl-gcc8.2")
        set(WITH_TRT ON)
    elseif(CUDA_VERSION EQUAL 10.2)
        set(CUDA_SUFFIX "cuda10.2-cudnn8-mkl-gcc8.2")
        set(WITH_TRT ON)
    elseif(CUDA_VERSION EQUAL 10.1)
        set(CUDA_SUFFIX "cuda10.1-cudnn7-mkl-gcc8.2")
        set(WITH_TRT ON)
    elseif(CUDA_VERSION EQUAL 10.0)
        set(CUDA_SUFFIX "cuda10-cudnn7-avx-mkl")
    elseif(CUDA_VERSION EQUAL 9.0)
        set(CUDA_SUFFIX "cuda9-cudnn7-avx-mkl")
    endif()
else()
    set(WITH_TRT OFF)
endif()  

if (WITH_GPU)
    SET(PADDLE_LIB_VERSION "${PADDLE_VERSION}-gpu-${CUDA_SUFFIX}")
elseif (WITH_LITE)
    if (WITH_XPU)
        SET(PADDLE_LIB_VERSION "${PADDLE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}-xpu")
    else()
        SET(PADDLE_LIB_VERSION "${PADDLE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
    endif()
else()
    if (WITH_AVX)
        if (WITH_MKLML)
            SET(PADDLE_LIB_VERSION "${PADDLE_VERSION}-cpu-avx-mkl")
        else()
            SET(PADDLE_LIB_VERSION "${PADDLE_VERSION}-cpu-avx-openblas")
        endif()
    else()
        SET(PADDLE_LIB_VERSION "${PADDLE_VERSION}-cpu-noavx-openblas")
    endif()
endif()

if(WITH_LITE)
    SET(PADDLE_LIB_PATH "http://paddle-serving.bj.bcebos.com/inferlib/${PADDLE_LIB_VERSION}/paddle_inference.tgz")
else()
    SET(PADDLE_LIB_PATH "http://paddle-inference-lib.bj.bcebos.com/${PADDLE_LIB_VERSION}/paddle_inference.tgz")
endif()

MESSAGE(STATUS "PADDLE_LIB_PATH=${PADDLE_LIB_PATH}")
if (WITH_GPU OR WITH_MKLML)
    if (WITH_TRT)
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
                ${CMAKE_COMMAND} -E copy_directory ${PADDLE_DOWNLOAD_DIR}/third_party ${PADDLE_INSTALL_DIR}/third_party &&
                ${CMAKE_COMMAND} -E copy ${PADDLE_INSTALL_DIR}/third_party/install/mkldnn/lib/libdnnl.so.1 ${PADDLE_INSTALL_DIR}/third_party/install/mkldnn/lib/libdnnl.so
        )
    else()
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
                ${CMAKE_COMMAND} -E copy_directory ${PADDLE_DOWNLOAD_DIR}/third_party ${PADDLE_INSTALL_DIR}/third_party &&
                ${CMAKE_COMMAND} -E copy ${PADDLE_INSTALL_DIR}/third_party/install/mkldnn/lib/libmkldnn.so.0 ${PADDLE_INSTALL_DIR}/third_party/install/mkldnn/lib/libmkldnn.so 
        )
    endif()
else()
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
endif()

INCLUDE_DIRECTORIES(${PADDLE_INCLUDE_DIR})
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}" "${PADDLE_INSTALL_DIR}/third_party/install/mklml/lib")
LINK_DIRECTORIES(${PADDLE_INSTALL_DIR}/third_party/install/mklml/lib)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}" "${PADDLE_INSTALL_DIR}/third_party/install/mkldnn/lib")
LINK_DIRECTORIES(${PADDLE_INSTALL_DIR}/third_party/install/mkldnn/lib)

if (NOT WITH_MKLML)
    ADD_LIBRARY(openblas STATIC IMPORTED GLOBAL)
    SET_PROPERTY(TARGET openblas PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/third_party/install/openblas/lib/libopenblas.a)
endif()

ADD_LIBRARY(paddle_inference STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET paddle_inference PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/lib/libpaddle_inference.a)

if (WITH_TRT)
    ADD_LIBRARY(nvinfer SHARED IMPORTED GLOBAL)
    SET_PROPERTY(TARGET nvinfer PROPERTY IMPORTED_LOCATION ${TENSORRT_ROOT}/lib/libnvinfer.so)
    
    ADD_LIBRARY(nvinfer_plugin SHARED IMPORTED GLOBAL)
    SET_PROPERTY(TARGET nvinfer_plugin PROPERTY IMPORTED_LOCATION ${TENSORRT_ROOT}/lib/libnvinfer_plugin.so)
endif()

if (WITH_LITE)
    ADD_LIBRARY(paddle_full_api_shared STATIC IMPORTED GLOBAL)
    SET_PROPERTY(TARGET paddle_full_api_shared PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/third_party/install/lite/cxx/lib/libpaddle_full_api_shared.so)
    
    if (WITH_XPU)
        ADD_LIBRARY(xpuapi SHARED IMPORTED GLOBAL)
        SET_PROPERTY(TARGET xpuapi PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/third_party/install/xpu/lib/libxpuapi.so)
    
        ADD_LIBRARY(xpurt SHARED IMPORTED GLOBAL)
        SET_PROPERTY(TARGET xpurt PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/third_party/install/xpu/lib/libxpurt.so)
    endif()
endif()

ADD_LIBRARY(xxhash STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET xxhash PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/third_party/install/xxhash/lib/libxxhash.a)

ADD_LIBRARY(cryptopp STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET cryptopp PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/third_party/install/cryptopp/lib/libcryptopp.a)

ADD_LIBRARY(gloo STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET gloo PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/third_party/install/gloo/lib/libgloo.a)

LIST(APPEND external_project_dependencies paddle)

LIST(APPEND paddle_depend_libs
        xxhash cryptopp gloo)

if(WITH_LITE)
    LIST(APPEND paddle_depend_libs paddle_full_api_shared)
    if(WITH_XPU)
        LIST(APPEND paddle_depend_libs xpuapi xpurt)
    endif()
endif()

if(WITH_TRT)
    LIST(APPEND paddle_depend_libs
        nvinfer nvinfer_plugin)
endif()
