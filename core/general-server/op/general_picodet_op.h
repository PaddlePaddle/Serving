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

#pragma once
#include <string>
#include <vector>
#include <numeric>

#include "core/general-server/general_model_service.pb.h"
#include "core/general-server/op/general_infer_helper.h"

#include "core/predictor/tools/pp_shitu_tools/preprocess_op.h"
#include "paddle_inference_api.h"  // NOLINT

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

namespace baidu {
namespace paddle_serving {
namespace serving {

struct ObjectResult {
    // Rectangle coordinates of detected object: left, right, top, down
    std::vector<int> rect;
    // Class id of detected object
    int class_id;
    // Confidence of detected object
    float confidence;
};

class ImageBlob {
public:
    // image width and height
    std::vector<float> im_shape_;
    // Buffer for image data after preprocessing
    std::vector<float> im_data_;
    // in net data shape(after pad)
    std::vector<float> in_net_shape_;
    // Scale factor for image size to origin image size
    std::vector<float> scale_factor_;
};

class GeneralPicodetOp
    : public baidu::paddle_serving::predictor::OpWithChannel<GeneralBlob> {
  public:
    typedef std::vector<paddle::PaddleTensor> TensorVector;
    DECLARE_OP(GeneralPicodetOp);
    int inference();   //op to do inference

  private:
    // rec preprocess
    std::vector<float> mean_ = {0.485f, 0.456f, 0.406f};
    std::vector<float> std_ = {0.229f, 0.224f, 0.225f};
    float scale_ = 0.00392157;
    int resize_size_  = 224;
    int resize_short_ = 224;

    Feature::ResizeImg resize_op_;
    Feature::Normalize normalize_op_;
    Feature::Permute permute_op_;

  private:
    // det pre-process
    ImageBlob imgblob;

    //resize
    int  interp_ = 2;
    bool keep_ratio_ = false;
    std::vector<int> target_size_ = {640,640};
    std::vector<int> in_net_shape_;

    void InitInfo_Run(cv::Mat *im, ImageBlob *data) {
        data->im_shape_ = {static_cast<float>(im->rows),
                           static_cast<float>(im->cols)};
        data->scale_factor_ = {1., 1.};
        data->in_net_shape_ = {static_cast<float>(im->rows),
                               static_cast<float>(im->cols)};
    }

    void NormalizeImage_Run(cv::Mat *im, ImageBlob *data) {
        double e = 1.0;
        e /= 255.0;
        (*im).convertTo(*im, CV_32FC3, e);
        for (int h = 0; h < im->rows; h++) {
            for (int w = 0; w < im->cols; w++) {
                im->at<cv::Vec3f>(h, w)[0] =
                        (im->at<cv::Vec3f>(h, w)[0] - mean_[0]) / std_[0];
                im->at<cv::Vec3f>(h, w)[1] =
                        (im->at<cv::Vec3f>(h, w)[1] - mean_[1]) / std_[1];
                im->at<cv::Vec3f>(h, w)[2] =
                        (im->at<cv::Vec3f>(h, w)[2] - mean_[2]) / std_[2];
            }
        }
        VLOG(2) << "enter NormalizeImage_Run run";
        VLOG(2) << data->im_shape_[0];
        VLOG(2) << data->im_shape_[1];
        VLOG(2) << data->scale_factor_[0];
        VLOG(2) << data->scale_factor_[1];
    }

    void Resize_Run(cv::Mat *im, ImageBlob *data) {
        auto resize_scale = GenerateScale(*im);
        data->im_shape_ = {static_cast<float>(im->cols * resize_scale.first),
                           static_cast<float>(im->rows * resize_scale.second)};
        data->in_net_shape_ = {static_cast<float>(im->cols * resize_scale.first),
                               static_cast<float>(im->rows * resize_scale.second)};
        cv::resize(*im, *im, cv::Size(), resize_scale.first, resize_scale.second,
                   interp_);
        data->im_shape_ = {
                static_cast<float>(im->rows), static_cast<float>(im->cols),
        };
        data->scale_factor_ = {
                resize_scale.second, resize_scale.first,
        };
        VLOG(2) << "enter resize run";
        VLOG(2) << data->im_shape_[0];
        VLOG(2) << data->im_shape_[1];
        VLOG(2) << data->scale_factor_[0];
        VLOG(2) << data->scale_factor_[1];
    }

    std::pair<double, double> GenerateScale(const cv::Mat &im) {
        std::pair<double, double> resize_scale;
        int origin_w = im.cols;
        int origin_h = im.rows;

        if (keep_ratio_) {
            int im_size_max = std::max(origin_w, origin_h);
            int im_size_min = std::min(origin_w, origin_h);
            int target_size_max =
                    *std::max_element(target_size_.begin(), target_size_.end());
            int target_size_min =
                    *std::min_element(target_size_.begin(), target_size_.end());
            double scale_min =
                    static_cast<double>(target_size_min) / static_cast<double>(im_size_min);
            double scale_max =
                    static_cast<double>(target_size_max) / static_cast<double>(im_size_max);
            double scale_ratio = std::min(scale_min, scale_max);
            resize_scale = {scale_ratio, scale_ratio};
        } else {
            resize_scale.first =
                    static_cast<double>(target_size_[1]) / static_cast<double>(origin_w);
            resize_scale.second =
                    static_cast<double>(target_size_[0]) / static_cast<double>(origin_h);
        }
        return resize_scale;
    }

    void Permute_Run(cv::Mat *im, ImageBlob *data) {
        int rh = im->rows;
        int rw = im->cols;
        int rc = im->channels();
        (data->im_data_).resize(rc * rh * rw);
        float *base = (data->im_data_).data();
        for (int i = 0; i < rc; ++i) {
            cv::extractChannel(*im, cv::Mat(rh, rw, CV_32FC1, base + i * rh * rw), i);
        } 
    }

    //det process
    int max_detect_results = 5;
    void Postprocess(const std::vector <cv::Mat> mats,
                                     std::vector <ObjectResult> *result,
                                     std::vector<int> bbox_num,
                                     bool is_rbox,
                                     std::vector<float> output_data_,
                                     std::vector<int> out_bbox_num_data_);
}; // GeneralPicodetOp
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
