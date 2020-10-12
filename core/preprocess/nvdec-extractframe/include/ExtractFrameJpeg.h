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

#include <nvjpeg.h>

#include <string>

#include "ExtractFrameBase.h"
#ifndef CORE_PREPROCESS_NVDEC_EXTRACTFRAME_INCLUDE_EXTRACTFRAMEJPEG_H_
#define CORE_PREPROCESS_NVDEC_EXTRACTFRAME_INCLUDE_EXTRACTFRAMEJPEG_H_

namespace baidu {
namespace xvision {
class ExtractFrameJpeg : public ExtractFrameBase {
  /**
   * @name:
   *     ExtractFrameJpeg
   * @author:
   *     chengang06@baidu.com
   * @main feature:
   *     extract video and output jpeg format data
   * @example:
   *
   **/

 public:
  explicit ExtractFrameJpeg(int gpu_index) : ExtractFrameBase(gpu_index) {}
  ExtractFrameJpeg() {}
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
  IMGDataList extract_frame(const std::string& file_path, int n = 1);

  int init();
  virtual ~ExtractFrameJpeg() {
    if (_nv_enc_params) {
      nvjpegEncoderParamsDestroy(_nv_enc_params);
    }
    if (_nv_enc_state) {
      nvjpegEncoderStateDestroy(_nv_enc_state);
    }
    if (_nv_jpeg_handler) {
      nvjpegDestroy(_nv_jpeg_handler);
    }
    if (_cuda_stream) {
      cudaStreamDestroy(_cuda_stream);
    }
  }

 private:
  /**
   * @Name:
   *     jpeg_encode
   * @Feature:
   *     use cuda to encode jpeg picture
   * @params
   *     p_image, rgba raw pointer, can be memory on gpu card
   *     width, height: image size info
   *     FrameResult result, output, jpeg picture
   * @returns
   *
   **/
  int jpeg_encode(uint8_t* p_image,
                  int width,
                  int height,
                  const FrameResult& result);
  nvjpegHandle_t _nv_jpeg_handler;
  nvjpegEncoderState_t _nv_enc_state;
  nvjpegEncoderParams_t _nv_enc_params;
  cudaStream_t _cuda_stream;
};
}  // namespace xvision
}  // namespace baidu
#endif  // CORE_PREPROCESS_NVDEC_EXTRACTFRAME_INCLUDE_EXTRACTFRAMEJPEG_H_
