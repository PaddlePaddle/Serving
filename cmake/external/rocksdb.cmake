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

SET(ROCKSDB_SOURCES_DIR ${THIRD_PARTY_PATH}/rocksdb)
SET(ROCKSDB_DOWNLOAD_DIR ${ROCKSDB_SOURCES_DIR}/src/extern_rocksdb)
SET(ROCKSDB_INSTALL_DIR ${THIRD_PARTY_PATH}/install/rocksdb/)
SET(ROCKSDB_INCLUDE_DIR "${ROCKSDB_INSTALL_DIR}/include" CACHE PATH "RocksDB include directory." FORCE)
SET(ROCKSDB_LIBRARIES "${ROCKSDB_INSTALL_DIR}/lib/librocksdb.a" CACHE FILEPATH "RocksDB library." FORCE)

message("rocksdb install dir: " ${ROCKSDB_INSTALL_DIR})

# Reference https://stackoverflow.com/questions/45414507/pass-a-list-of-prefix-paths-to-externalproject-add-in-cmake-args
set(prefix_path "${THIRD_PARTY_PATH}/install/gflags|${THIRD_PARTY_PATH}/install/leveldb|${THIRD_PARTY_PATH}/install/snappy|${THIRD_PARTY_PATH}/install/gtest|${THIRD_PARTY_PATH}/install/protobuf|${THIRD_PARTY_PATH}/install/zlib|${THIRD_PARTY_PATH}/install/glog")

SET(ROCKSDB_VERSION "6.1")
SET(ROCKSDB_LIB_PATH "http://paddle-serving.bj.bcebos.com/dev/gcc485/rocksdb.tar.gz")

ExternalProject_Add(
    "extern_rocksdb"
    ${EXTERNAL_PROJECT_LOG_ARGS}
    URL                 "${ROCKSDB_LIB_PATH}"
    PREFIX              "${ROCKSDB_SOURCES_DIR}"
    DOWNLOAD_DIR        "${ROCKSDB_DOWNLOAD_DIR}"
    CONFIGURE_COMMAND   ""
    BUILD_COMMAND       ""
    UPDATE_COMMAND      ""
    INSTALL_COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${ROCKSDB_DOWNLOAD_DIR}/include ${ROCKSDB_INSTALL_DIR}/include &&
        ${CMAKE_COMMAND} -E copy_directory ${ROCKSDB_DOWNLOAD_DIR}/lib ${ROCKSDB_INSTALL_DIR}/lib 
)
INCLUDE_DIRECTORIES(${ROCKSDB_INCLUDE_DIR})
ADD_LIBRARY(rocksdb STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET rocksdb PROPERTY IMPORTED_LOCATION ${ROCKSDB_INSTALL_DIR}/lib/librocksdb.a)
