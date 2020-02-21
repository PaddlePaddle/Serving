# Copyright (c) 2016 PaddlePaddle Authors. All Rights Reserved.
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

SET(GTEST_SOURCES_DIR ${THIRD_PARTY_PATH}/gtest)
SET(GTEST_INSTALL_DIR ${THIRD_PARTY_PATH}/install/gtest)
SET(GTEST_INCLUDE_DIR "${GTEST_INSTALL_DIR}/include" CACHE PATH "gtest include directory." FORCE)
SET(GTEST_LIBRARIES "${GTEST_INSTALL_DIR}/lib/libgtest.a" CACHE FILEPATH "gtest library." FORCE)
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})

ExternalProject_Add(
    extern_gtest
    ${EXTERNAL_PROJECT_LOG_ARGS}
    PREFIX ${GTEST_SOURCES_DIR}
    GIT_REPOSITORY "https://github.com/google/googletest"
    GIT_TAG master
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND CXXFLAGS=-fPIC && mkdir -p build && cd build && cmake .. && make -j ${NUM_OF_PROCESSOR} gtest
    INSTALL_COMMAND mkdir -p ${GTEST_INSTALL_DIR}/lib/ 
    && cp ${GTEST_SOURCES_DIR}/src/extern_gtest/build/lib/libgtest.a ${GTEST_LIBRARIES}
    && cp -r ${GTEST_SOURCES_DIR}/src/extern_gtest/googletest/include ${GTEST_INSTALL_DIR}/
    BUILD_IN_SOURCE 1
)

ADD_DEPENDENCIES(extern_gtest snappy)

ADD_LIBRARY(gtest STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET gtest PROPERTY IMPORTED_LOCATION ${GTEST_LIBRARIES})

LIST(APPEND external_project_dependencies gtest)
