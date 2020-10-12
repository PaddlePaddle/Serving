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

#include "ExtractFrameBGRARaw.h"
#include <nppi.h>
#include <memory>
#include <string>
baidu::xvision::IMGDataList baidu::xvision::ExtractFrameBGRARaw::extract_frame(
    const std::string &file_path, int n) {
  FFmpegDemuxer demuxer(file_path.c_str());
  NvDecoder nvdec(
      p_cu_context, true, FFmpeg2NvCodecId(demuxer.GetVideoCodec()));
  size_t n_width = demuxer.GetWidth();
  size_t n_height = demuxer.GetHeight();
  double frame_rate = demuxer.GetFrameRate();
  size_t n_frame_size = n_height * n_width * 4;
  uint8_t *p_video = nullptr;
  uint8_t **pp_frame = nullptr;
  CUdeviceptr p_tmp_image = 0;
  cuMemAlloc(&p_tmp_image, n_frame_size);

  int n_video_bytes = 0;
  int frame_count = -1;
  int frame_returned = 0;
  int64_t cur_frame_time(0), pre_frame_time(0), pts(0);
  int64_t *p_timestamp = nullptr;
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
      fm_tmp.set_rows(n_height);
      fm_tmp.set_cols(n_width);
      fm_tmp.set_thick(4);
      fm_tmp.set_frame_buffer(new uint8_t[n_frame_size]);
      fm_tmp.set_height(n_height);
      fm_tmp.set_width(n_width);
      result_list.push_back(fm_tmp);
      if (nvdec.GetBitDepth() == 8) {
        if (nvdec.GetOutputFormat() == cudaVideoSurfaceFormat_YUV444) {
          YUV444ToColor32<BGRA32>(reinterpret_cast<uint8_t *>(pp_frame[i]),
                                  nvdec.GetWidth(),
                                  reinterpret_cast<uint8_t *>(p_tmp_image),
                                  4 * nvdec.GetWidth(),
                                  nvdec.GetWidth(),
                                  nvdec.GetHeight());
        } else {
          Nv12ToColor32<BGRA32>(reinterpret_cast<uint8_t *>(pp_frame[i]),
                                nvdec.GetWidth(),
                                reinterpret_cast<uint8_t *>(p_tmp_image),
                                4 * nvdec.GetWidth(),
                                nvdec.GetWidth(),
                                nvdec.GetHeight());
        }
        GetImage(p_tmp_image,
                 fm_tmp.get_frame(),
                 nvdec.GetWidth(),
                 4 * nvdec.GetHeight());
      } else {
        if (nvdec.GetOutputFormat() == cudaVideoSurfaceFormat_YUV444_16Bit) {
          YUV444P16ToColor32<BGRA32>(reinterpret_cast<uint8_t *>(pp_frame[i]),
                                     2 * nvdec.GetWidth(),
                                     reinterpret_cast<uint8_t *>(p_tmp_image),
                                     4 * nvdec.GetWidth(),
                                     nvdec.GetWidth(),
                                     nvdec.GetHeight());
        } else {
          P016ToColor32<BGRA32>(reinterpret_cast<uint8_t *>(pp_frame[i]),
                                nvdec.GetWidth(),
                                reinterpret_cast<uint8_t *>(p_tmp_image),
                                4 * nvdec.GetWidth(),
                                nvdec.GetWidth(),
                                nvdec.GetHeight());
        }
        GetImage(p_tmp_image,
                 fm_tmp.get_frame(),
                 nvdec.GetWidth(),
                 4 * nvdec.GetHeight());
      }
      /*GetImage((CUdeviceptr) pp_frame[i], reinterpret_cast<uint8_t
         *>(fm_tmp.p_frame.get()),
               nvdec.GetWidth(),
               nvdec.GetHeight() + (nvdec.GetChromaHeight() *
         nvdec.GetNumChromaPlanes()));*/
    }
  } while (n_video_bytes);
  cuMemFree(p_tmp_image);
  return result_list;
}
