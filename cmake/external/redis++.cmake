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

SET(REDISCLIENT_SOURCES_DIR ${THIRD_PARTY_PATH}/redis++)
SET(REDISCLIENT_INSTALL_DIR ${THIRD_PARTY_PATH}/install/redis++)
SET(REDISCLIENT_INCLUDE_DIR "${REDISCLIENT_INSTALL_DIR}/include" CACHE PATH "redis++ include directory." FORCE)
SET(REDISCLIENT_LIBRARIES "${REDISCLIENT_INSTALL_DIR}/lib/libredis++.a" CACHE FILEPATH "redis++ library." FORCE)
INCLUDE_DIRECTORIES(${REDISCLIENT_INCLUDE_DIR})

ExternalProject_Add(
    extern_redis++
    ${EXTERNAL_PROJECT_LOG_ARGS}
    PREFIX ${REDISCLIENT_SOURCES_DIR}
    GIT_REPOSITORY "https://github.com/sewenew/redis-plus-plus"
    GIT_TAG master
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND cmake . && CXXFLAGS=-fPIC make -j ${NUM_OF_PROCESSOR} static
    INSTALL_COMMAND mkdir -p ${REDISCLIENT_INSTALL_DIR}/lib/ 
    && cp ${REDISCLIENT_SOURCES_DIR}/src/extern_redis++/lib/libredis++.a ${REDISCLIENT_LIBRARIES}
    && cp -r ${REDISCLIENT_SOURCES_DIR}/src/extern_redis++/src/sw/redis++/ ${REDISCLIENT_INSTALL_DIR}/include/redis++/
    BUILD_IN_SOURCE 1
)

ADD_DEPENDENCIES(extern_redis++ snappy)

ADD_LIBRARY(redis++ STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET redis++ PROPERTY IMPORTED_LOCATION ${ROCKSDB_LIBRARIES})
ADD_DEPENDENCIES(redis++ extern_redis++)

LIST(APPEND external_project_dependencies redis++)

