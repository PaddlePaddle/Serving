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

SET(JEMALLOC_SOURCES_DIR ${THIRD_PARTY_PATH}/jemalloc)
SET(JEMALLOC_INSTALL_DIR ${THIRD_PARTY_PATH}/install/jemalloc)
SET(JEMALLOC_INCLUDE_DIR "${JEMALLOC_INSTALL_DIR}/include" CACHE PATH "jemalloc include directory." FORCE)

ExternalProject_Add(
        extern_jemalloc
        ${EXTERNAL_PROJECT_LOG_ARGS}
        GIT_REPOSITORY  "https://github.com/jemalloc/jemalloc.git"
        GIT_TAG         "5.2.1"
        PREFIX          ${JEMALLOC_SOURCES_DIR}
        CONFIGURE_COMMAND ""
        BUILD_COMMAND   ""
        INSTALL_COMMAND cd ${JEMALLOC_SOURCES_DIR}/src/extern_jemalloc/ && sh autogen.sh
            && make
            && mkdir -p ${JEMALLOC_INSTALL_DIR}/lib/
            && cp ${JEMALLOC_SOURCES_DIR}/src/extern_jemalloc/lib/libjemalloc.a ${JEMALLOC_INSTALL_DIR}/lib
            && cp ${JEMALLOC_SOURCES_DIR}/src/extern_jemalloc/lib/libjemalloc_pic.a ${JEMALLOC_INSTALL_DIR}/lib
            && cp -r ${JEMALLOC_SOURCES_DIR}/src/extern_jemalloc/include/jemalloc  ${JEMALLOC_INCLUDE_DIR}
        TEST_COMMAND      ""
)