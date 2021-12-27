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

#include "core/general-server/op/general_picodet_op.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>

#include "core/predictor/framework/infer.h"
#include "core/predictor/framework/memory.h"
#include "core/predictor/framework/resource.h"
#include "core/util/include/timer.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::Timer;
using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::Response;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::InferManager;
using baidu::paddle_serving::predictor::PaddleGeneralModelConfig;

int GeneralPicodetOp::inference() {
  VLOG(2) << "Going to run inference";
  const std::vector<std::string> pre_node_names = pre_names();
  if (pre_node_names.size() != 1) {
    LOG(ERROR) << "This op(" << op_name()
               << ") can only have one predecessor op, but received "
               << pre_node_names.size();
    return -1;
  }
  const std::string pre_name = pre_node_names[0];

  const GeneralBlob* input_blob = get_depend_argument<GeneralBlob>(pre_name);
  if (!input_blob) {
    LOG(ERROR) << "input_blob is nullptr,error";
    return -1;
  }
  uint64_t log_id = input_blob->GetLogId();
  VLOG(2) << "(logid=" << log_id << ") Get precedent op name: " << pre_name;

  GeneralBlob* output_blob = mutable_data<GeneralBlob>();
  if (!output_blob) {
    LOG(ERROR) << "output_blob is nullptr,error";
    return -1;
  }
  output_blob->SetLogId(log_id);

  if (!input_blob) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Failed mutable depended argument, op:" << pre_name;
    return -1;
  }

  const TensorVector* in = &input_blob->tensor_vector;
  TensorVector* out = &output_blob->tensor_vector;
  int batch_size = input_blob->_batch_size;
  VLOG(2) << "(logid=" << log_id << ") input batch size: " << batch_size;
  output_blob->_batch_size = batch_size;

  //get image shape
  float* data = (float*)in->at(0).data.data();
  int height  = data[0];
  int width   = data[1];
  VLOG(2) << "image width: " << width;
  VLOG(2) << "image height: " << height;

  ///////////////////det preprocess begin/////////////////////////
  //show raw image
  unsigned char* img_data = static_cast<unsigned char*>(in->at(1).data.data());
  cv::Mat origin(height, width, CV_8UC3, img_data);
  // cv::imwrite("/workspace/origin_image.jpg", origin);

  cv::Mat origin_img = origin.clone();
  cv::cvtColor(origin, origin, cv::COLOR_BGR2RGB);  
  InitInfo_Run(&origin, &imgblob); 
  Resize_Run(&origin, &imgblob);
  NormalizeImage_Run(&origin, &imgblob);
  Permute_Run(&origin, &imgblob);
  ///////////////////det preprocess end/////////////////////////

  Timer timeline;
  int64_t start = timeline.TimeStampUS();
  timeline.Start();

  //generate real_in
  TensorVector* real_in = new TensorVector();
  if (!real_in) {
    LOG(ERROR) << "real_in is nullptr, error";
    return -1;
  }

  //generate im_shape
  int in_num = 2;
  size_t databuf_size = in_num * sizeof(float);
  void *databuf_data = MempoolWrapper::instance().malloc(databuf_size);
  if (!databuf_data) {
    LOG(ERROR) << "Malloc failed, size: " << databuf_size;
    return -1;
  }
  float* databuf_float = reinterpret_cast<float*>(databuf_data);
  *databuf_float = imgblob.im_shape_[0];
  databuf_float++;
  *databuf_float = imgblob.im_shape_[1];

  char* databuf_char = reinterpret_cast<char*>(databuf_data);
  paddle::PaddleBuf paddleBuf(databuf_char, databuf_size);
  paddle::PaddleTensor tensor_in;
  tensor_in.name = "im_shape";
  tensor_in.dtype = paddle::PaddleDType::FLOAT32;
  tensor_in.shape = {1, 2};
  tensor_in.lod = in->at(0).lod;
  tensor_in.data = paddleBuf;
  real_in->push_back(tensor_in);

  //generate scale_factor
  databuf_size = in_num * sizeof(float);
  databuf_data = MempoolWrapper::instance().malloc(databuf_size);
  if (!databuf_data) {
    LOG(ERROR) << "Malloc failed, size: " << databuf_size;
    return -1;
  }
  databuf_float = reinterpret_cast<float*>(databuf_data);
  *databuf_float = imgblob.scale_factor_[0];
  databuf_float++;
  *databuf_float = imgblob.scale_factor_[1];
  databuf_char = reinterpret_cast<char*>(databuf_data);
  paddle::PaddleBuf paddleBuf_2(databuf_char, databuf_size);
  paddle::PaddleTensor tensor_in_2;
  tensor_in_2.name = "scale_factor";
  tensor_in_2.dtype = paddle::PaddleDType::FLOAT32;
  tensor_in_2.shape = {1, 2};
  tensor_in_2.lod = in->at(0).lod;
  tensor_in_2.data = paddleBuf_2;
  real_in->push_back(tensor_in_2);

  //genarate image
  in_num = imgblob.im_data_.size();
  databuf_size = in_num * sizeof(float);
  databuf_data = MempoolWrapper::instance().malloc(databuf_size);
  if (!databuf_data) {
    LOG(ERROR) << "Malloc failed, size: " << databuf_size;
    return -1;
  }
  memcpy(databuf_data, imgblob.im_data_.data(), databuf_size);
  databuf_char = reinterpret_cast<char*>(databuf_data);
  paddle::PaddleBuf paddleBuf_3(databuf_char, databuf_size);
  paddle::PaddleTensor tensor_in_3;
  tensor_in_3.name = "image";
  tensor_in_3.dtype = paddle::PaddleDType::FLOAT32;
  tensor_in_3.shape = {1, 3, imgblob.in_net_shape_[0],  imgblob.in_net_shape_[1]};
  tensor_in_3.lod = in->at(0).lod;
  tensor_in_3.data = paddleBuf_3;
  real_in->push_back(tensor_in_3);

  if (InferManager::instance().infer(
          engine_name().c_str(), real_in, out, batch_size)) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Failed do infer in fluid model: " << engine_name().c_str();
    return -1;
  }

  ///////////////////det postprocess begin/////////////////////////
  //get output_data_
  std::vector<float>  output_data_;
  int infer_outnum = out->size();
  paddle::PaddleTensor element = out->at(0);
  std::vector<int> element_shape = element.shape;
  //get data len
  int total_num = 1;
  for(auto value_shape: element_shape)
  {
      total_num *= value_shape;
  }
  output_data_.resize(total_num);

  float* data_out = (float*)element.data.data();
  for(int j=0; j < total_num; j++)
  {
    output_data_[j] = data_out[j];
  }

  //det postprocess
  //1) get detect result
  if(output_data_.size() > max_detect_results * 6){
    output_data_.resize(max_detect_results * 6);
  }
  std::vector <ObjectResult> result;
  int detect_num = output_data_.size() / 6;
  for(int m = 0; m < detect_num; m++)
  {
      // Class id
      int class_id = static_cast<int>(round(output_data_[0 + m * 6]));
      // Confidence score
      float score = output_data_[1 + m * 6];
      // Box coordinate
      int xmin = (output_data_[2 + m * 6]);
      int ymin = (output_data_[3 + m * 6]);
      int xmax = (output_data_[4 + m * 6]);
      int ymax = (output_data_[5 + m * 6]);

      ObjectResult result_item;
      result_item.rect = {xmin, ymin, xmax, ymax};
      result_item.class_id = class_id;
      result_item.confidence = score;
      result.push_back(result_item);
  }

  //2) add the whole image
  ObjectResult result_whole_img = {
          {0, 0, width - 1, height - 1}, 0, 1.0}; 
  result.push_back(result_whole_img);


  //3) crop image and do preprocess. concanate the data
  cv::Mat srcimg;
  cv::cvtColor(origin_img, srcimg, cv::COLOR_BGR2RGB);
  std::vector<float> all_data;
  for (int j = 0; j < result.size(); ++j) {
      int w = result[j].rect[2] - result[j].rect[0];  
      int h = result[j].rect[3] - result[j].rect[1];    
      cv::Rect rect(result[j].rect[0], result[j].rect[1], w, h);
      cv::Mat crop_img = srcimg(rect);
      cv::Mat resize_img;
      resize_op_.Run(crop_img, resize_img, resize_short_, resize_size_);
      normalize_op_.Run(&resize_img, mean_, std_, scale_);
      std::vector<float> input(1 * 3 * resize_img.rows * resize_img.cols, 0.0f);
      permute_op_.Run(&resize_img, input.data());
      for(int m = 0; m < input.size(); m++)
      {
        all_data.push_back(input[m]);
      }
  }
  ///////////////////det postprocess begin/////////////////////////

  //generate new Tensors;
  //"x"
  int out_num = all_data.size();
  int databuf_size_out = out_num * sizeof(float);
  void *databuf_data_out = MempoolWrapper::instance().malloc(databuf_size_out);
  if (!databuf_data_out) {
    LOG(ERROR) << "Malloc failed, size: " << databuf_size_out;
    return -1;
  }
  memcpy(databuf_data_out, all_data.data(), databuf_size_out);
  char *databuf_char_out = reinterpret_cast<char*>(databuf_data_out);
  paddle::PaddleBuf paddleBuf_out(databuf_char_out, databuf_size_out);
  paddle::PaddleTensor tensor_out;

  tensor_out.name = "x";
  tensor_out.dtype = paddle::PaddleDType::FLOAT32;
  tensor_out.shape = {result.size(), 3, 224, 224};
  tensor_out.data = paddleBuf_out;
  tensor_out.lod = in->at(0).lod;
  out->push_back(tensor_out);

  //"boxes"
  int box_size_out = result.size() * 6 * sizeof(float);
  void *box_data_out = MempoolWrapper::instance().malloc(box_size_out);
  if (!box_data_out) {
    LOG(ERROR) << "Malloc failed, size: " << box_data_out;
    return -1;
  }
  memcpy(box_data_out, out->at(0).data.data(), box_size_out - 6 * sizeof(float));
  float *box_float_out = reinterpret_cast<float *>(box_data_out);
  box_float_out += (result.size() - 1) * 6;
  box_float_out[0] = 0.0;
  box_float_out[1] = 1.0;
  box_float_out[2] = 0.0;
  box_float_out[3] = 0.0;
  box_float_out[4] = width - 1;
  box_float_out[5] = height - 1;
  char *box_char_out = reinterpret_cast<char*>(box_data_out);
  paddle::PaddleBuf paddleBuf_out_2(box_char_out, box_size_out);
  paddle::PaddleTensor tensor_out_2;

  tensor_out_2.name = "boxes";
  tensor_out_2.dtype = paddle::PaddleDType::FLOAT32;
  tensor_out_2.shape = {result.size(), 6};
  tensor_out_2.data = paddleBuf_out_2;
  tensor_out_2.lod = in->at(0).lod;
  out->push_back(tensor_out_2);
  out->erase(out->begin(), out->begin() + infer_outnum);

  int64_t end = timeline.TimeStampUS();
  CopyBlobInfo(input_blob, output_blob);
  AddBlobInfo(output_blob, start);
  AddBlobInfo(output_blob, end);
  return 0;
}

DEFINE_OP(GeneralPicodetOp);
 
void GeneralPicodetOp::Postprocess(const std::vector <cv::Mat> mats,
                                     std::vector <ObjectResult> *result,
                                     std::vector<int> bbox_num,
                                     bool is_rbox, 
                                     std::vector<float> output_data_,
                                     std::vector<int> out_bbox_num_data_){
        result->clear();
        int start_idx = 0;
        for (int im_id = 0; im_id < mats.size(); im_id++) {
            cv::Mat raw_mat = mats[im_id];
            int rh = 1;
            int rw = 1;
            for (int j = start_idx; j < start_idx + bbox_num[im_id]; j++) {
                if (is_rbox) {
                    // Class id + score + 8 parameters 
                    // Class id
                    int class_id = static_cast<int>(round(output_data_[0 + j * 10]));
                    // Confidence score
                    float score = output_data_[1 + j * 10];
                    int x1 = (output_data_[2 + j * 10] * rw);
                    int y1 = (output_data_[3 + j * 10] * rh);
                    int x2 = (output_data_[4 + j * 10] * rw);
                    int y2 = (output_data_[5 + j * 10] * rh);
                    int x3 = (output_data_[6 + j * 10] * rw);
                    int y3 = (output_data_[7 + j * 10] * rh);
                    int x4 = (output_data_[8 + j * 10] * rw);
                    int y4 = (output_data_[9 + j * 10] * rh);

                    ObjectResult result_item;
                    result_item.rect = {x1, y1, x2, y2, x3, y3, x4, y4};
                    result_item.class_id = class_id;
                    result_item.confidence = score;
                    result->push_back(result_item);
                } else {
                    // Class id
                    int class_id = static_cast<int>(round(output_data_[0 + j * 6]));
                    // Confidence score
                    float score = output_data_[1 + j * 6];
                    
                    //xmin, ymin, xmax, ymax
                    int xmin = (output_data_[2 + j * 6] * rw);
                    int ymin = (output_data_[3 + j * 6] * rh);
                    int xmax = (output_data_[4 + j * 6] * rw);
                    int ymax = (output_data_[5 + j * 6] * rh);

                    //get width; get height
                    int wd = xmax - xmin; //width
                    int hd = ymax - ymin; //height

                    ObjectResult result_item;
                    result_item.rect = {xmin, ymin, xmax, ymax};
                    result_item.class_id = class_id;
                    result_item.confidence = score;
                    result->push_back(result_item);
                }
            }
            start_idx += bbox_num[im_id];
        }
}
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
