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

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(cube-transfer Go)

include(cmake/golang.cmake)

ExternalGoProject_Add(rfw github.com/mipearson/rfw)
ExternalGoProject_Add_With_Deps(docopt-go github.com/docopt/docopt-go github.com/mipearson/rfw)
ExternalGoProject_Add_With_Deps(logex github.com/Badangel/logex github.com/mipearson/rfw)

add_subdirectory(src)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION ${PADDLE_SERVING_INSTALL_DIR})
