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
 * @file ExtractFrameBase.h
 * @author chengang06@baidu.com
 * @date 2020-04-15
 **/
#include <cuda.h>

#include <list>
#include <memory>
#include <string>

#include "NvDecoder/NvDecoder.h"
#include "Utils/ColorSpace.h"
#include "Utils/FFmpegDemuxer.h"
#ifndef CORE_PREPROCESS_NVDEC_EXTRACTFRAME_INCLUDE_EXTRACTFRAMEBASE_H_
#define CORE_PREPROCESS_NVDEC_EXTRACTFRAME_INCLUDE_EXTRACTFRAMEBASE_H_
namespace baidu {
namespace xvision {

class FrameResult {
  /**
   * @name:
   *     FrameResult
   * @author:
   *     chengang06@baidu.com
   * @main feature:
   *     store image info
   * @example:
   *     None
   **/
 public:
  // constructor
  FrameResult() {
    _height = 0;
    _width = 0;
    _p_frame = nullptr;
    _cols = 0;
    _rows = 0;
    _thick = 0;
  }
  /*
  FrameResult deepcopy() const {
      FrameResult tmp;
      std::cout << "copy constructor called" << std::endl;
      tmp.set_height(_height);
      tmp.set_width(_width);
      tmp.set_cols(_cols);
      tmp.set_rows(_rows);
      tmp.set_thick(_thick);

      auto frame_tmp = new uint8_t(this -> get_frame_buff_len());
      std::copy(this -> get_frame(), this -> get_frame() + this ->
  get_frame_buff_len(), frame_tmp);
      tmp.set_frame_buffer(frame_tmp);

  }
  */
  // attributes
  size_t rows() const { return _rows; }
  size_t cols() const { return _cols; }
  size_t thick() const { return _thick; }
  size_t height() const { return _height; }
  size_t width() const { return _width; }
  size_t len() const { return _rows * _cols * _thick; }
  void set_rows(const size_t rows) { _rows = rows; }
  void set_cols(const size_t cols) { _cols = cols; }
  void set_thick(const size_t thick) { _thick = thick; }
  void set_width(const size_t width) { _width = width; }
  void set_height(const size_t height) { _height = height; }
  // free buffer data manually

  void free_memory() {
    if (_p_frame) {
      delete[] _p_frame;
      _p_frame = nullptr;
    }
    _height = 0;
    _width = 0;
    _rows = 0;
    _cols = 0;
    _thick = 0;
  }
  // set frame buffer
  void set_frame_buffer(uint8_t* buff) {
    if (_p_frame) {
      delete[] _p_frame;
      _p_frame = nullptr;
    }
    _p_frame = buff;
  }
  // get frame buffer
  uint8_t* get_frame() const { return _p_frame; }
  size_t get_frame_buff_len() const { return _rows * _cols * _thick; }
  virtual ~FrameResult() {}

 private:
  uint8_t* _p_frame;
  size_t _height, _width;       // pic width and height
  size_t _rows, _cols, _thick;  // buffer sizes
};
typedef std::list<FrameResult> IMGDataList;
class ExtractFrameBase {
  /**
   * @name:
   *     ExtractFrameBase
   * @author:
   *     chengang06@baidu.com
   * @main feature:
   *     base class for extract frame
   * @example:
   *
   **/
 public:
  explicit ExtractFrameBase(int gpu_index) {
    this->gpu_index = gpu_index;
    p_cu_context = nullptr;
  }
  ExtractFrameBase() {
    gpu_index = 0;
    p_cu_context = nullptr;
  }
  virtual int init();
  virtual IMGDataList extract_frame(const std::string& file_path,
                                    int n = 1,
                                    int count = 200) = 0;
  virtual ~ExtractFrameBase() {
    if (p_cu_context != nullptr) {
      cuCtxDestroy(p_cu_context);
    }
  }
  /**
   * @name select_frame
   * @param, frame_rate, double like 25.00,
   * @param, pre_frame_time, int_64, last selected frame timestamp
   * @param, cur_frame_time, int_64, current frame timestamp
   * @param, fps, extract frame num per seconds
   * @param, mode, 0, use time stamp to select frame, 1, use framerate to select
   *frame
   **/
  static bool select_frame(const double frame_rate,
                           const int64_t pre_frame_time,
                           const int64_t cur_frame_time,
                           const size_t frame_index,
                           const double fps,
                           const int mode);

 protected:
  int gpu_index;  // gpu index
  CUcontext p_cu_context;
};
}  // namespace xvision
}  // namespace baidu
#endif  // CORE_PREPROCESS_NVDEC_EXTRACTFRAME_INCLUDE_EXTRACTFRAMEBASE_H_
