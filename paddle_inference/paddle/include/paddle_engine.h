// Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
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

#include <dirent.h>
#include <pthread.h>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "core/configure/include/configure_parser.h"
#include "core/configure/inferencer_configure.pb.h"
#include "core/predictor/common/utils.h"
#include "core/predictor/framework/infer.h"
#include "paddle_inference_api.h"  // NOLINT

namespace baidu {
namespace paddle_serving {
namespace inference {

using paddle_infer::Config;
using paddle_infer::PrecisionType;
using paddle_infer::Predictor;
using paddle_infer::Tensor;
using paddle_infer::CreatePredictor;

DECLARE_int32(gpuid);
DECLARE_string(precision);
DECLARE_bool(use_calib);

static const int max_batch = 32;
static const int min_subgraph_size = 3;
static PrecisionType precision_type;

std::shared_ptr<std::vector<paddle::PaddleTensor>> PrepareWarmupData() {
  auto warmup_data = std::make_shared<std::vector<paddle::PaddleTensor>>(1);
  paddle::PaddleTensor images;
  images.name = "image";
  images.shape = {2, 3, 300, 300};
  images.dtype = paddle::PaddleDType::FLOAT32;
  images.data.Resize(sizeof(float) * 2 * 3 * 300 * 300);

  (*warmup_data)[0] = std::move(images);
  return warmup_data;
}

PrecisionType GetPrecision(const std::string& precision_data) {
  std::string precision_type = predictor::ToLower(precision_data);
  if (precision_type == "fp32") {
    return PrecisionType::kFloat32;
  } else if (precision_type == "int8") {
    return PrecisionType::kInt8;
  } else if (precision_type == "fp16") {
    return PrecisionType::kHalf;
  }
  return PrecisionType::kFloat32;
}

const std::string getFileBySuffix(
    const std::string& path, const std::vector<std::string>& suffixVector) {
  DIR* dp = nullptr;
  std::string fileName = "";
  struct dirent* dirp = nullptr;
  if ((dp = opendir(path.c_str())) == nullptr) {
    return fileName;
  }
  while ((dirp = readdir(dp)) != nullptr) {
    if (dirp->d_type == DT_REG) {
      for (int idx = 0; idx < suffixVector.size(); ++idx) {
        if (std::string(dirp->d_name).find(suffixVector[idx]) !=
            std::string::npos) {
          fileName = static_cast<std::string>(dirp->d_name);
          break;
        }
      }
    }
    if (fileName.length() != 0) break;
  }
  closedir(dp);
  return fileName;
}

// Engine Core is the base class of inference engines, which can be derived from
// paddle Inference Engine, or inference engines of other machine learning
// platforms
class EngineCore {
 public:
  virtual ~EngineCore() {}
  virtual std::vector<std::string> GetInputNames() {
    return _predictor->GetInputNames();
  }

  virtual std::unique_ptr<Tensor> GetInputHandle(const std::string& name) {
    return _predictor->GetInputHandle(name);
  }

  virtual std::vector<std::string> GetOutputNames() {
    return _predictor->GetOutputNames();
  }

  virtual std::unique_ptr<Tensor> GetOutputHandle(const std::string& name) {
    return _predictor->GetOutputHandle(name);
  }

  virtual bool Run() {
    if (!_predictor->Run()) {
      LOG(ERROR) << "Failed call Run with paddle predictor";
      return false;
    }
    return true;
  }

  virtual int create(const configure::EngineDesc& conf, int gpu_id) = 0;

  virtual int clone(void* predictor) {
    if (predictor == NULL) {
      LOG(ERROR) << "origin paddle Predictor is null.";
      return -1;
    }
    Predictor* prep = static_cast<Predictor*>(predictor);
    _predictor = prep->Clone();
    if (_predictor.get() == NULL) {
      LOG(ERROR) << "fail to clone paddle predictor: " << predictor;
      return -1;
    }
    return 0;
  }

  virtual void* get() { return _predictor.get(); }

 protected:
  // _predictor is a prediction instance of Paddle Inference.
  // when inferring on the CPU, _predictor is bound to a model.
  // when inferring on the GPU, _predictor is bound to a model and a GPU card.
  // Therefore, when using GPU multi-card inference, you need to create multiple
  // EngineCore.
  std::shared_ptr<Predictor> _predictor;
};

// Paddle Inference Engine
class PaddleInferenceEngine : public EngineCore {
 public:
  int create(const configure::EngineDesc& engine_conf, int gpu_id) {
    std::string model_path = engine_conf.model_dir();
    if (access(model_path.c_str(), F_OK) == -1) {
      LOG(ERROR) << "create paddle predictor failed, path not exits: "
                 << model_path;
      return -1;
    }

    Config config;
    std::vector<std::string> suffixParaVector = {".pdiparams", "__params__"};
    std::vector<std::string> suffixModelVector = {".pdmodel", "__model__"};
    std::string paraFileName = getFileBySuffix(model_path, suffixParaVector);
    std::string modelFileName = getFileBySuffix(model_path, suffixModelVector);

    std::string encryParaPath = model_path + "/encrypt_model";
    std::string encryModelPath = model_path + "/encrypt_params";
    std::string encryKeyPath = model_path + "/key";

    // encrypt model
    if (access(encryParaPath.c_str(), F_OK) != -1 &&
        access(encryModelPath.c_str(), F_OK) != -1 &&
        access(encryKeyPath.c_str(), F_OK) != -1) {
      // decrypt model

      std::string model_buffer, params_buffer, key_buffer;
      predictor::ReadBinaryFile(model_path + "/encrypt_model", &model_buffer);
      predictor::ReadBinaryFile(model_path + "/encrypt_params", &params_buffer);
      predictor::ReadBinaryFile(model_path + "/key", &key_buffer);

      auto cipher = paddle::MakeCipher("");
      std::string real_model_buffer = cipher->Decrypt(model_buffer, key_buffer);
      std::string real_params_buffer =
          cipher->Decrypt(params_buffer, key_buffer);
      config.SetModelBuffer(&real_model_buffer[0],
                            real_model_buffer.size(),
                            &real_params_buffer[0],
                            real_params_buffer.size());
    } else if (paraFileName.length() != 0 && modelFileName.length() != 0) {
      config.SetParamsFile(model_path + "/" + paraFileName);
      config.SetProgFile(model_path + "/" + modelFileName);
    } else {
      config.SetModel(model_path);
    }

    config.SwitchSpecifyInputNames(true);
    config.SetCpuMathLibraryNumThreads(1);
    if (engine_conf.has_use_gpu() && engine_conf.use_gpu()) {
      // 2000MB GPU memory
      config.EnableUseGpu(50, gpu_id);
      if (engine_conf.has_gpu_multi_stream() &&
          engine_conf.gpu_multi_stream()) {
        config.EnableGpuMultiStream();
      }
    }
    precision_type = GetPrecision(FLAGS_precision);

    if (engine_conf.has_enable_ir_optimization() &&
        !engine_conf.enable_ir_optimization()) {
      config.SwitchIrOptim(false);
    } else {
      config.SwitchIrOptim(true);
    }

    if (engine_conf.has_use_trt() && engine_conf.use_trt()) {
      config.SwitchIrOptim(true);
      if (!engine_conf.has_use_gpu() || !engine_conf.use_gpu()) {
        config.EnableUseGpu(50, gpu_id);
        if (engine_conf.has_gpu_multi_stream() &&
            engine_conf.gpu_multi_stream()) {
          config.EnableGpuMultiStream();
        }
      }
      config.EnableTensorRtEngine(1 << 30,
                                  max_batch,
                                  min_subgraph_size,
                                  precision_type,
                                  true,
                                  FLAGS_use_calib);
      // set trt dynamic shape
      {
        int bsz = 1;
        int max_seq_len = 512;
        std::map<std::string, std::vector<int>> min_input_shape;
        std::map<std::string, std::vector<int>> max_input_shape;
        std::map<std::string, std::vector<int>> optim_input_shape;
        int hidden_size = 0;

        min_input_shape["stack_0.tmp_0"] = {1, 16, 1, 1};
        min_input_shape["stack_1.tmp_0"] = {1, 2, 1, 1};
        min_input_shape["input_mask"] = {1, 1, 1};
        min_input_shape["_generated_var_63"] = {1, 1, 12288};
        min_input_shape["tmp_127"] = {1, 1, 12288};
        min_input_shape["_generated_var_87"] = {1, 1, 768};
        min_input_shape["tmp_175"] = {1, 1, 768};

        max_input_shape["stack_0.tmp_0"] = {bsz, 16, max_seq_len, max_seq_len};
        max_input_shape["stack_1.tmp_0"] = {bsz, 2, max_seq_len, max_seq_len};
        max_input_shape["input_mask"] = {bsz, max_seq_len, max_seq_len};
        max_input_shape["_generated_var_63"] = {bsz, max_seq_len, 12288};
        max_input_shape["tmp_127"] = {bsz, max_seq_len, 12288};
        max_input_shape["_generated_var_87"] = {bsz, max_seq_len, 768};
        max_input_shape["tmp_175"] = {bsz, max_seq_len, 768};

        for (int i = 0; i < 44; ++i) {
          if (i >= 32) {
            hidden_size = 768;
          } else {
            hidden_size = 12288;
          }
          int i1 = 3 * i;
          int i2 = 3 * i + 1;
          int i3 = 3 * i + 2;
          char name[256];
          for (int j = 0; j < 3; ++j) {
            snprintf(name, 256, "c_identity_%d.tmp_0", 3 * i + j);
            min_input_shape[name] = {1, 1, hidden_size};
            max_input_shape[name] = {bsz, max_seq_len, hidden_size};
          }
        }
        optim_input_shape = max_input_shape;
        config.SetTRTDynamicShapeInfo(
            min_input_shape, max_input_shape, optim_input_shape);
        config.Exp_DisableTensorRtOPs({"layer_norm"});
      }
      LOG(INFO) << "create TensorRT predictor";
    }

    if (engine_conf.has_use_lite() && engine_conf.use_lite()) {
      config.EnableLiteEngine(precision_type, true);
    }

    if ((!engine_conf.has_use_lite() && !engine_conf.has_use_gpu()) ||
        (engine_conf.has_use_lite() && !engine_conf.use_lite() &&
         engine_conf.has_use_gpu() && !engine_conf.use_gpu())) {
#ifdef WITH_MKLML
#ifdef WITH_MKLDNN
      config.EnableMKLDNN();
      config.SwitchIrOptim(true);
      config.DisableGpu();
      // config.SetCpuMathLibraryNumThreads(2);

      if (precision_type == PrecisionType::kInt8) {
        config.EnableMkldnnQuantizer();
        auto quantizer_config = config.mkldnn_quantizer_config();
        // TODO(somebody): warmup data
        // quantizer_config -> SetWarmupData();
        // quantizer_config -> SetWarmupBatchSize();
        // quantizer_config -> SetEnabledOpTypes(4);
      } else if (precision_type == PrecisionType::kHalf) {
        config.EnableMkldnnBfloat16();
      }
#endif
#endif
    }

    if (engine_conf.has_use_xpu() && engine_conf.use_xpu()) {
      // 2 MB l3 cache
      config.EnableXpu(2 * 1024 * 1024);
    }

    if (engine_conf.has_enable_memory_optimization() &&
        engine_conf.enable_memory_optimization()) {
      config.EnableMemoryOptim();
    }

    predictor::AutoLock lock(predictor::GlobalCreateMutex::instance());
    _predictor = CreatePredictor(config);
    if (NULL == _predictor.get()) {
      LOG(ERROR) << "create paddle predictor failed, path: " << model_path;
      return -1;
    }

    VLOG(2) << "create paddle predictor sucess, path: " << model_path;
    return 0;
  }
};

}  // namespace inference
}  // namespace paddle_serving
}  // namespace baidu
