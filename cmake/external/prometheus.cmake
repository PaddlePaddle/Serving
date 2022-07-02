# Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
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
SET(GIT_URL https://github.com)
SET(PROMETHEUS_PREFIX_DIR    ${THIRD_PARTY_PATH}/prometheus)
SET(PROMETHEUS_INSTALL_DIR   ${THIRD_PARTY_PATH}/install/prometheus)
SET(PROMETHEUS_REPOSITORY    ${GIT_URL}/jupp0r/prometheus-cpp.git)
SET(PROMETHEUS_TAG           v0.13.0)

INCLUDE_DIRECTORIES(${PROMETHEUS_INSTALL_DIR}/include)

ExternalProject_Add(
  extern_prometheus
  ${EXTERNAL_PROJECT_LOG_ARGS}
  ${SHALLOW_CLONE}
  GIT_REPOSITORY        ${PROMETHEUS_REPOSITORY}
  GIT_TAG               ${PROMETHEUS_TAG}
  PREFIX                ${PROMETHEUS_PREFIX_DIR}
  UPDATE_COMMAND        ""
  CMAKE_ARGS            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                        -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
                        -DCMAKE_C_FLAGS_DEBUG=${CMAKE_C_FLAGS_DEBUG}
                        -DCMAKE_C_FLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE}
                        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
                        -DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
                        -DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}
                        -DCMAKE_INSTALL_PREFIX:PATH=${PROMETHEUS_INSTALL_DIR}
                        -DCMAKE_INSTALL_LIBDIR=${PROMETHEUS_INSTALL_DIR}/lib
                        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                        -DBUILD_SHARED_LIBS=OFF
                        -DENABLE_PUSH=OFF
                        -DENABLE_COMPRESSION=OFF
                        -DENABLE_TESTING=OFF
  BUILD_BYPRODUCTS     ${PROMETHEUS_LIBRARIES}
)

ADD_LIBRARY(prometheus STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET prometheus PROPERTY IMPORTED_LOCATION ${PROMETHEUS_INSTALL_DIR}/lib/libprometheus-cpp-core.a)
ADD_LIBRARY(prometheus-pull STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET prometheus-pull PROPERTY IMPORTED_LOCATION ${PROMETHEUS_INSTALL_DIR}/lib/libprometheus-cpp-pull.a)
ADD_DEPENDENCIES(prometheus extern_prometheus)
LIST(APPEND prometheus_libs prometheus-pull)