// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
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

#include "examples/demo-serving/op/reader_op.h"
#include <algorithm>
#include "core/predictor/framework/memory.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::format::XImageReqInstance;
using baidu::paddle_serving::predictor::image_classification::Request;

int ReaderOp::inference() {
  const Request* req = dynamic_cast<const Request*>(get_request_message());
  LOG(INFO) << "Receive request in dense service:" << req->ShortDebugString();

  ReaderOutput* res = mutable_data<ReaderOutput>();
  if (!res) {
    LOG(ERROR) << "Failed get op tls reader object output";
    return -1;
  }

  TensorVector* in = &res->tensors;
  uint32_t sample_size = req->instances_size();
  if (sample_size <= 0) {
    LOG(WARNING) << "No instances need to inference!";
    return -1;
  }

  // TODO(xxx) pmeans/scales/isize/enable_crop should be configurable.
  float pmean[3] = {0.485 * 255, 0.456 * 255, 0.406 * 255};
  float scale[3] = {1 / (0.229 * 255), 1 / (0.224 * 255), 1 / (0.225 * 255)};
  size_t iresize[] = {244, 244};  // row, column
  bool enable_crop = true;

  cv::Size resize;
  resize.height = iresize[0];
  resize.width = iresize[1];

  paddle::PaddleTensor in_tensor;
  in_tensor.name = "tensor";
  in_tensor.dtype = paddle::FLOAT32;
  // shape assignment
  in_tensor.shape.push_back(sample_size);  // batch_size
  in_tensor.shape.push_back(3);
  in_tensor.shape.push_back(resize.width);
  in_tensor.shape.push_back(resize.height);

  // tls resource assignment
  size_t dense_capacity = 3 * resize.width * resize.height;
  size_t len = dense_capacity * sizeof(float) * sample_size;

  // Allocate buffer in PaddleTensor, so that buffer will be managed by the
  // Tensor
  in_tensor.data.Resize(len);
  float* data = reinterpret_cast<float*>(in_tensor.data.data());
  if (in_tensor.data.data() == NULL) {
    LOG(ERROR) << "Failed create temp float array, "
               << "size=" << dense_capacity * sample_size * sizeof(float);
    return -1;
  }

  for (uint32_t si = 0; si < sample_size; si++) {
    // parse image object from x-image
    const XImageReqInstance& ins = req->instances(si);
    // read dense image from request bytes
    const char* binary = ins.image_binary().c_str();
    size_t length = ins.image_length();
    if (length == 0) {
      LOG(ERROR) << "Empty image, length is 0";
      return -1;
    }

    _image_vec_tmp.clear();
    _image_vec_tmp.assign(binary, binary + length);
    _image_8u_tmp = cv::imdecode(cv::Mat(_image_vec_tmp),
                                 CV_LOAD_IMAGE_COLOR /*1*/);  // in B/G/R order.
    if (_image_8u_tmp.data == NULL) {
      LOG(ERROR) << "Image decode failed!";
      return -1;
    }

    // accumulate length
    const int HH = _image_8u_tmp.rows;
    const int WW = _image_8u_tmp.cols;
    const int CC = _image_8u_tmp.channels();

    // resize/crop
    if (_image_8u_tmp.cols != resize.width ||
        _image_8u_tmp.rows != resize.height) {
      int short_egde = std::min<int>(_image_8u_tmp.cols, _image_8u_tmp.rows);
      int yy = static_cast<int>((_image_8u_tmp.rows - short_egde) / 2);
      int xx = static_cast<int>((_image_8u_tmp.cols - short_egde) / 2);
      _image_8u_tmp =
          cv::Mat(_image_8u_tmp, cv::Rect(xx, yy, short_egde, short_egde));
      if (_image_8u_tmp.cols != resize.width ||
          _image_8u_tmp.rows != resize.height) {
        cv::Mat resize_image;
        cv::resize(_image_8u_tmp, resize_image, resize);
        _image_8u_tmp = resize_image;
      }

      LOG(INFO) << "Succ crop one image[CHW=" << _image_8u_tmp.channels()
                << ", " << _image_8u_tmp.cols << ", " << _image_8u_tmp.rows
                << "]"
                << " from image[CHW=" << CC << ", " << HH << ", " << WW << "]";
    }

    // BGR->RGB transformer
    cv::cvtColor(_image_8u_tmp, _image_8u_rgb, CV_BGR2RGB);

    const int H = _image_8u_rgb.rows;
    const int W = _image_8u_rgb.cols;
    const int C = _image_8u_rgb.channels();
    if (H != resize.height || W != resize.width || C != 3) {
      LOG(ERROR) << "Image " << si << " has incompitable size";
      return -1;
    }

    LOG(INFO) << "Succ read one image, C: " << C << ", W: " << W
              << ", H: " << H;

    float* data_ptr = data + dense_capacity * si;
    for (int h = 0; h < H; h++) {
      // p points to a new line
      unsigned char* p = _image_8u_rgb.ptr<unsigned char>(h);
      for (int w = 0; w < W; w++) {
        for (int c = 0; c < C; c++) {
          // HWC(row,column,channel) -> CWH
          data_ptr[W * H * c + W * h + w] =
              (p[C * w + c] - pmean[c]) * scale[c];
        }
      }
    }
  }
  in->push_back(in_tensor);

  return 0;
}

DEFINE_OP(ReaderOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
