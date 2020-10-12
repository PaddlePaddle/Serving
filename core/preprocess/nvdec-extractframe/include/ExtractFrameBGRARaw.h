// Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file ExtractFrameBGRARaw.h
 * @author chengang06@baidu.com
 * @date 2020-04-15
 **/
#ifndef CORE_PREPROCESS_NVDEC_EXTRACTFRAME_INCLUDE_EXTRACTFRAMEBGRARAW_H_
#define CORE_PREPROCESS_NVDEC_EXTRACTFRAME_INCLUDE_EXTRACTFRAMEBGRARAW_H_
#include <string>

#include "ExtractFrameBase.h"
namespace baidu {
namespace xvision {
class ExtractFrameBGRARaw : public ExtractFrameBase {
  /**
   * @name:
   *     ExtractFrameBGRARaw
   * @author:
   *     chengang06@baidu.com
   * @main feature:
   *     extract video and output bgr raw data
   * @example:
   *
   **/
 public:
  explicit ExtractFrameBGRARaw(int gpu_index) : ExtractFrameBase(gpu_index) {}
  ExtractFrameBGRARaw() {}
  /**
   * @Name:
   *     extract_frame
   * @Feature:
   *     extract video frame frames from video file,
   * @params
   *     file_path: video local path
   *     n:         n frames per second
   * @returns
   *     IMGDataList
   **/
  IMGDataList extract_frame(const std::string &file_path,
                            int n = 1,
                            int count = 200);
};
}  // namespace xvision
}  // namespace baidu
#endif  // CORE_PREPROCESS_NVDEC_EXTRACTFRAME_INCLUDE_EXTRACTFRAMEBGRARAW_H_
