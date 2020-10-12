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

#include "ExtractFrameJpeg.h"

#include <nppi.h>

#include <memory>
#include <string>

int baidu::xvision::ExtractFrameJpeg::init() {
  auto result = ExtractFrameBase::init();
  if (result != 0) {
    return result;
  }
  auto cuda_init = cudaStreamCreate(&_cuda_stream);
  if (cuda_init != ::cudaError::cudaSuccess) {
    return -2;
  }
  _nv_jpeg_handler = nullptr;
  _nv_enc_state = nullptr;
  _nv_enc_params = nullptr;
  nvjpegStatus_t stats = nvjpegCreateSimple(&_nv_jpeg_handler);
  // nvjpeg initialization error return -2
  if (stats != nvjpegStatus_t::NVJPEG_STATUS_SUCCESS) {
    return -2;
  }
  stats = nvjpegEncoderParamsCreate(
      _nv_jpeg_handler, &_nv_enc_params, _cuda_stream);
  if (stats != nvjpegStatus_t::NVJPEG_STATUS_SUCCESS) {
    return -2;
  }
  // high quality
  stats = nvjpegEncoderParamsSetQuality(_nv_enc_params, 100, _cuda_stream);
  if (stats != nvjpegStatus_t::NVJPEG_STATUS_SUCCESS) {
    return -2;
  }
  stats =
      nvjpegEncoderStateCreate(_nv_jpeg_handler, &_nv_enc_state, _cuda_stream);
  if (stats != nvjpegStatus_t::NVJPEG_STATUS_SUCCESS) {
    return -2;
  }
  stats = nvjpegEncoderParamsSetSamplingFactors(
      _nv_enc_params, NVJPEG_CSS_444, _cuda_stream);
  if (stats != nvjpegStatus_t::NVJPEG_STATUS_SUCCESS) {
    return -2;
  }
  return 0;
}
baidu::xvision::IMGDataList baidu::xvision::ExtractFrameJpeg::extract_frame(
    const std::string &file_path, int n) {
  FFmpegDemuxer demuxer(file_path.c_str());
  NvDecoder nvdec(
      p_cu_context, true, FFmpeg2NvCodecId(demuxer.GetVideoCodec()));
  int n_width = demuxer.GetWidth();
  int n_height = demuxer.GetHeight();
  double frame_rate = demuxer.GetFrameRate();
  int n_frame_size = n_width * n_height * 4;  // rgbp depth 3 uint_8
  uint8_t *p_video = nullptr;
  uint8_t **pp_frame = nullptr;
  int64_t *p_timestamp = nullptr;
  CUdeviceptr p_tmp_image = 0;
  cuMemAlloc(&p_tmp_image, n_frame_size);
  int n_video_bytes = 0;
  int frame_count = -1;
  int frame_returned = 0;
  int64_t pts = 0;
  int64_t pre_frame_time = 0;
  int64_t cur_frame_time = 0;
  if (n == 0) {
    n = 1000;
  }
  IMGDataList result_list;
  do {
    demuxer.Demux(&p_video, &n_video_bytes, &pts);

    nvdec.Decode(p_video,
                 n_video_bytes,
                 &pp_frame,
                 &frame_returned,
                 0,
                 &p_timestamp,
                 pts);
    for (auto i = 0; i < frame_returned; ++i) {
      cur_frame_time = p_timestamp[i];
      frame_count += 1;
      if (!select_frame(
              frame_rate, pre_frame_time, cur_frame_time, frame_count, n, 0)) {
        continue;
      }
      pre_frame_time = cur_frame_time;

      FrameResult fm_tmp;
      fm_tmp.set_height(nvdec.GetHeight());
      fm_tmp.set_width(nvdec.GetWidth());
      if (nvdec.GetBitDepth() == 8) {
        if (nvdec.GetOutputFormat() == cudaVideoSurfaceFormat_YUV444) {
          YUV444ToColorPlanar<RGBA32>(reinterpret_cast<uint8_t *>(pp_frame[i]),
                                      nvdec.GetWidth(),
                                      reinterpret_cast<uint8_t *>(p_tmp_image),
                                      nvdec.GetWidth(),
                                      nvdec.GetWidth(),
                                      nvdec.GetHeight());
        } else {
          Nv12ToColorPlanar<RGBA32>(reinterpret_cast<uint8_t *>(pp_frame[i]),
                                    nvdec.GetWidth(),
                                    reinterpret_cast<uint8_t *>(p_tmp_image),
                                    nvdec.GetWidth(),
                                    nvdec.GetWidth(),
                                    nvdec.GetHeight());
        }
      } else {
        if (nvdec.GetOutputFormat() == cudaVideoSurfaceFormat_YUV444_16Bit)
          YUV444P16ToColorPlanar<RGBA32>(
              reinterpret_cast<uint8_t *>(pp_frame[i]),
              2 * nvdec.GetWidth(),
              reinterpret_cast<uint8_t *>(p_tmp_image),
              nvdec.GetWidth(),
              nvdec.GetWidth(),
              nvdec.GetHeight());
        else
          P016ToColorPlanar<RGBA32>(reinterpret_cast<uint8_t *>(pp_frame[i]),
                                    2 * nvdec.GetWidth(),
                                    reinterpret_cast<uint8_t *>(p_tmp_image),
                                    nvdec.GetWidth(),
                                    nvdec.GetWidth(),
                                    nvdec.GetHeight());
      }
      jpeg_encode(reinterpret_cast<uint8_t *>(p_tmp_image),
                  nvdec.GetWidth(),
                  nvdec.GetHeight(),
                  fm_tmp);
      result_list.push_back(fm_tmp);
    }
  } while (n_video_bytes);
  cuMemFree(p_tmp_image);
  return result_list;
}
int baidu::xvision::ExtractFrameJpeg::jpeg_encode(uint8_t *p_image,
                                                  int width,
                                                  int height,
                                                  FrameResult &result) {
  nvjpegImage_t nv_image;
  nv_image.channel[0] = p_image;
  nv_image.channel[1] = p_image + width * height;
  nv_image.channel[2] = p_image + width * height * 2;
  nv_image.pitch[0] = width;
  nv_image.pitch[1] = width;
  nv_image.pitch[2] = width;
  auto stat = nvjpegEncodeImage(_nv_jpeg_handler,
                                _nv_enc_state,
                                _nv_enc_params,
                                &nv_image,
                                NVJPEG_INPUT_RGB,
                                width,
                                height,
                                _cuda_stream);

  if (stat != nvjpegStatus_t::NVJPEG_STATUS_SUCCESS) {
    return -1;
  }
  size_t len(0);
  stat = nvjpegEncodeRetrieveBitstream(
      _nv_jpeg_handler, _nv_enc_state, nullptr, &len, _cuda_stream);

  auto cuda_stat = cudaStreamSynchronize(_cuda_stream);

  if (cuda_stat != ::cudaSuccess) {
    return -1;
  }
  result.set_frame_buffer(new uint8_t[len]);
  result.set_width(width);
  result.set_height(height);
  result.set_rows(len);
  result.set_cols(1);
  result.set_thick(1);
  // jpeg, rows: len, cols:1, thick:height
  stat = nvjpegEncodeRetrieveBitstream(
      _nv_jpeg_handler, _nv_enc_state, result.get_frame(), &len, _cuda_stream);

  if (stat != nvjpegStatus_t::NVJPEG_STATUS_SUCCESS) {
    return -1;
  }
  cuda_stat = cudaStreamSynchronize(_cuda_stream);
  if (cuda_stat != ::cudaSuccess) {
    return -1;
  }
  return 0;
}
