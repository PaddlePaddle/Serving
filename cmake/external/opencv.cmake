# Copyright (c) 2018 PaddlePaddle Authors. All Rights Reserved.
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

include (ExternalProject)

# NOTE: opencv is needed when linking with paddle-serving example

set(OPENCV_SOURCES_DIR ${THIRD_PARTY_PATH}/opencv)
set(OPENCV_INSTALL_DIR ${THIRD_PARTY_PATH}/install/opencv)
set(OPENCV_INCLUDE_DIR "${OPENCV_INSTALL_DIR}/include" CACHE PATH "Opencv include directory." FORCE)
INCLUDE_DIRECTORIES(${OPENCV_INCLUDE_DIR})
ExternalProject_Add(
    extern_opencv
    GIT_REPOSITORY "https://github.com/opencv/opencv"
    GIT_TAG "3.2.0"
    PREFIX          ${OPENCV_SOURCES_DIR}
    UPDATE_COMMAND  ""
    PATCH_COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/cmake/patch/opencv_ippicv_downloader.cmake ${OPENCV_SOURCES_DIR}/src/extern_opencv/3rdparty/ippicv/downloader.cmake
    CMAKE_ARGS      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
                    -DCMAKE_C_FLAGS_DEBUG=${CMAKE_C_FLAGS_DEBUG}
                    -DCMAKE_C_FLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE}
                    -DCMAKE_CXX_FLAGS=${OPENCV_CMAKE_CXX_FLAGS}
                    -DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
                    -DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}
                    -DCMAKE_INSTALL_PREFIX=${OPENCV_INSTALL_DIR}
                    -DCMAKE_INSTALL_LIBDIR=${OPENCV_INSTALL_DIR}/lib
                    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                    -DWITH_GTK=OFF
                    -DBUILD_TESTS=OFF
                    -DBUILD_PERF_TESTS=OFF
                    -DCMAKE_BUILD_TYPE=${THIRD_PARTY_BUILD_TYPE}
                    -DWITH_EIGEN=OFF
                    -DWITH_CUDA=OFF
                    -DWITH_JPEG=ON
                    -DBUILD_JPEG=ON
                    -DWITH_PNG=ON
                    -DBUILD_PNG=ON
                    -DWITH_TIFF=ON
                    -DBUILD_TIFF=ON
                    -DBUILD_SHARED_LIBS=OFF
                    -DENABLE_PRECOMPILED_HEADERS=OFF
                    ${EXTERNAL_OPTIONAL_ARGS}
    CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${OPENCV_INSTALL_DIR}
                     -DCMAKE_INSTALL_LIBDIR:PATH=${OPENCV_INSTALL_DIR}/lib
                     -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
                     -DCMAKE_BUILD_TYPE:STRING=${THIRD_PARTY_BUILD_TYPE}
)
IF(WIN32)
    IF(NOT EXISTS "${OPENCV_INSTALL_DIR}/lib/libopencv_core.lib")
        add_custom_command(TARGET extern_opencv POST_BUILD
                COMMAND cmake -E copy ${OPENCV_INSTALL_DIR}/lib/opencv_core.lib ${OPENCV_INSTALL_DIR}/lib/libopencv_core.lib
                )
    ENDIF()

    IF(NOT EXISTS "${OPENCV_INSTALL_DIR}/lib/libopencv_imgcodecs.lib")
        add_custom_command(TARGET extern_opencv POST_BUILD
                COMMAND cmake -E copy ${OPENCV_INSTALL_DIR}/lib/opencv_imgcodecs.lib ${OPENCV_INSTALL_DIR}/lib/libopencv_imgcodecs.lib
                )
    ENDIF()

    IF(NOT EXISTS "${OPENCV_INSTALL_DIR}/lib/libopencv_imgproc.lib")
        add_custom_command(TARGET extern_opencv POST_BUILD
                COMMAND cmake -E copy ${OPENCV_INSTALL_DIR}/lib/opencv_imgproc.lib ${OPENCV_INSTALL_DIR}/lib/libopencv_imgproc.lib
                )
    ENDIF()

    set(OPENCV_CORE_LIB "${OPENCV_INSTALL_DIR}/lib/libopencv_core.lib")
    set(OPENCV_IMGCODECS_LIB "${OPENCV_INSTALL_DIR}/lib/libopencv_imgcodecs.lib")
    set(OPENCV_IMGPROC_LIB "${OPENCV_INSTALL_DIR}/lib/libopencv_imgproc.lib")
else(WIN32)
    set(OPENCV_CORE_LIB "${OPENCV_INSTALL_DIR}/lib/libopencv_core.a")
    set(OPENCV_IMGCODECS_LIB "${OPENCV_INSTALL_DIR}/lib/libopencv_imgcodecs.a")
    set(OPENCV_IMGPROC_LIB "${OPENCV_INSTALL_DIR}/lib/libopencv_imgproc.a")
endif (WIN32)

add_library(opencv_core STATIC IMPORTED GLOBAL)
set_property(TARGET opencv_core PROPERTY IMPORTED_LOCATION ${OPENCV_CORE_LIB})

add_library(opencv_imgcodecs STATIC IMPORTED GLOBAL)
set_property(TARGET opencv_imgcodecs PROPERTY IMPORTED_LOCATION ${OPENCV_IMGCODECS_LIB})

add_library(opencv_imgproc STATIC IMPORTED GLOBAL)
set_property(TARGET opencv_imgproc PROPERTY IMPORTED_LOCATION ${OPENCV_IMGPROC_LIB})


include_directories(${OPENCV_INCLUDE_DIR})
add_dependencies(opencv_core extern_opencv)

ExternalProject_Get_Property(extern_opencv BINARY_DIR)
ADD_LIBRARY(ippicv STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET ippicv PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/3rdparty/ippicv/ippicv_lnx/lib/intel64/libippicv.a)

ADD_LIBRARY(IlmImf STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET IlmImf PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/3rdparty/lib/libIlmImf.a)

ADD_LIBRARY(libjasper STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET libjasper PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/3rdparty/lib/liblibjasper.a)

ADD_LIBRARY(libwebp STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET libwebp PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/3rdparty/lib/liblibwebp.a)

ADD_LIBRARY(libjpeg STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET libjpeg PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/3rdparty/lib/liblibjpeg.a)

ADD_LIBRARY(libpng STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET libpng PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/3rdparty/lib/liblibpng.a)

ADD_LIBRARY(libtiff STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET libtiff PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/3rdparty/lib/liblibtiff.a)

#ADD_LIBRARY(zlib STATIC IMPORTED GLOBAL)
#SET_PROPERTY(TARGET zlib PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/3rdparty/lib/libzlib.a)

LIST(APPEND opencv_depend_libs
        opencv_imgproc
        opencv_core
        ippicv
        IlmImf
        libjasper
        libwebp
        libjpeg
        libpng
        libtiff
        zlib)
