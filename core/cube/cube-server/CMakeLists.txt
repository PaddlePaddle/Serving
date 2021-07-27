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
# limitations under the License

include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR}/../)

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER proto/cube.proto proto/control.proto)
# include PROTO_HEADER
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CPP_FLAGS} -DNDEBUG -O2 -D__const__= -pipe -W -Wall -Wno-unused-parameter -fPIC -fno-omit-frame-pointer")

if(CMAKE_VERSION VERSION_LESS "3.1.3")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
else()
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

set(SRC_LIST src/server.cpp 
    src/cube_bvar.cpp 
    src/dict.cpp
    src/dict_set.cpp 
    src/recycle.cpp 
    src/framework.cpp 
    src/control.cpp
    src/server.cpp)

add_executable(cube ${SRC_LIST} src/main.cpp
    ${PROTO_SRC} ${PROTO_HEADER})

set(DYNAMIC_LIB
    protobuf
    leveldb
    -lssl
    -lcrypto
    )

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(DYNAMIC_LIB ${DYNAMIC_LIB}
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework CoreData"
        "-framework CoreText"
        "-framework Security"
        "-framework Foundation"
        "-Wl,-U,_MallocExtension_ReleaseFreeMemory"
        "-Wl,-U,_ProfilerStart"
        "-Wl,-U,_ProfilerStop")
endif()

target_link_libraries(cube brpc ${DYNAMIC_LIB} -lpthread -ldl -lz)

#add_executable(cube_test ${SRC_LIST} test/cube_test.cpp
#    ${PROTO_SRC} ${PROTO_HEADER})

#target_link_libraries(cube_test brpc ${DYNAMIC_LIB} gtest -lpthread -ldl -lz)

# install
install(TARGETS cube
        RUNTIME DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/bin
        )
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR})
