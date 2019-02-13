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

SET(PCRE_SOURCES_DIR ${THIRD_PARTY_PATH}/pcre)
SET(PCRE_INSTALL_DIR ${THIRD_PARTY_PATH}/install/pcre)
SET(PCRE_ROOT ${PCRE_INSTALL_DIR} CACHE FILEPATH "pcre root directory." FORCE)
SET(PCRE_INCLUDE_DIR "${PCRE_INSTALL_DIR}/include" CACHE PATH "pcre include directory." FORCE)

INCLUDE_DIRECTORIES(${PCRE_INCLUDE_DIR}) # For pcre code to include its own headers.
INCLUDE_DIRECTORIES(${THIRD_PARTY_PATH}/install) # For Paddle code to include pcre.h.

ExternalProject_Add(
    extern_pcre
    ${EXTERNAL_PROJECT_LOG_ARGS}
    SVN_REPOSITORY  "svn://vcs.exim.org/pcre/code/tags/pcre-7.7"
    PREFIX          ${PCRE_SOURCES_DIR}
    UPDATE_COMMAND  ""
    PATCH_COMMAND  sh autogen.sh
    CONFIGURE_COMMAND ../extern_pcre/configure --prefix=${PCRE_INSTALL_DIR} --disable-shared --with-pic
    BUILD_COMMAND make
    INSTALL_COMMAND make install
    CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${PCRE_INSTALL_DIR}
                     -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
                     -DCMAKE_BUILD_TYPE:STRING=${THIRD_PARTY_BUILD_TYPE}
)
IF(WIN32)
  IF(NOT EXISTS "${PCRE_INSTALL_DIR}/lib/libz.lib")
    add_custom_command(TARGET extern_pcre POST_BUILD
            COMMAND cmake -E copy ${PCRE_INSTALL_DIR}/lib/pcrestatic.lib ${PCRE_INSTALL_DIR}/lib/libz.lib
            )
  ENDIF()
  SET(PCRE_LIBRARIES "${PCRE_INSTALL_DIR}/lib/libpcre.lib" CACHE FILEPATH "pcre library." FORCE)
ELSE(WIN32)
  SET(PCRE_LIBRARIES "${PCRE_INSTALL_DIR}/lib/libpcre.a" CACHE FILEPATH "pcre library." FORCE)
ENDIF(WIN32)

ADD_LIBRARY(pcre STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET pcre PROPERTY IMPORTED_LOCATION ${PCRE_LIBRARIES})
ADD_DEPENDENCIES(pcre extern_pcre)

LIST(APPEND external_project_dependencies pcre)
