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
# limitations under the License

set(CMAKE_CXX_STANDARD 11)

add_compile_options(-std=c++11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR}/../)

add_executable(cube-builder src/main.cpp include/cube-builder/util.h src/util.cpp src/builder_job.cpp include/cube-builder/builder_job.h include/cube-builder/define.h src/seqfile_reader.cpp include/cube-builder/seqfile_reader.h include/cube-builder/raw_reader.h include/cube-builder/vtext.h src/crovl_builder_increment.cpp include/cube-builder/crovl_builder_increment.h src/curl_simple.cpp include/cube-builder/curl_simple.h)

add_dependencies(cube-builder jsoncpp boost brpc)

set(DYNAMIC_LIB
    gflags
    jsoncpp
    brpc
    -lcurl
    -lssl
    -lcrypto
    ${CURL_LIB}
    )

target_link_libraries(cube-builder ${DYNAMIC_LIB})

# install
install(TARGETS cube-builder RUNTIME DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/bin)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/tool/kvtool.py DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/tool)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/tool/kv_to_seqfile.py DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/tool)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/tool/source DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/tool)
