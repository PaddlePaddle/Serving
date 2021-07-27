# Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

if(SERVER)
add_subdirectory(cube)
#add_subdirectory(kvdb)
endif()

if (CLIENT OR SERVER)
add_subdirectory(configure)
add_subdirectory(pdcodegen)
add_subdirectory(sdk-cpp)
endif()

if (APP)
add_subdirectory(configure)
add_subdirectory(preprocess)
endif()


if(CLIENT)
add_subdirectory(general-client)
endif()

if (SERVER)
add_subdirectory(predictor)
add_subdirectory(general-server)
endif()

if (CLIENT OR SERVER)
add_subdirectory(util)
endif()
