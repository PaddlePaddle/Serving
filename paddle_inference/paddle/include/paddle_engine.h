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
using paddle_infer::DistConfig;

DECLARE_int32(gpuid);
DECLARE_string(precision);
DECLARE_bool(use_calib);
DECLARE_string(nnadapter_device_names);
DECLARE_string(nnadapter_context_properties);
DECLARE_string(nnadapter_model_cache_dir);

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
        std::string fileName_in_Dir = static_cast<std::string>(dirp->d_name);
        if (fileName_in_Dir.length() >= suffixVector[idx].length() &&
            fileName_in_Dir.substr(
                fileName_in_Dir.length() - suffixVector[idx].length(),
                suffixVector[idx].length()) == suffixVector[idx]) {
          fileName = fileName_in_Dir;
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
    std::vector<std::string> suffixParaVector = {
        ".pdiparams", "__params__", "params"};
    std::vector<std::string> suffixModelVector = {
        ".pdmodel", "__model__", "model"};
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

    // Enable distributed model inferencing
    DistConfig distCfg;
    if (engine_conf.has_enable_dist_model() &&
        engine_conf.enable_dist_model()) {
      int ep_size = engine_conf.dist_endpoints_size();
      int cur_index = engine_conf.dist_subgraph_index();
      if (ep_size <= cur_index) {
        LOG(ERROR) << "create paddle predictor failed, Distributed model error."
                   << " dist_endpoints_size=" << ep_size
                   << " is not bigger than dist_subgraph_index=" << cur_index;
        return -1;
      }
      std::vector<std::string> vec_eps;
      for (int i = 0; i < ep_size; ++i) {
        vec_eps.emplace_back(engine_conf.dist_endpoints(i));
      }
      distCfg.EnableDistModel(true);
      distCfg.SetCarrierId(engine_conf.dist_carrier_id());
      distCfg.SetRanks(engine_conf.dist_nranks(), cur_index);
      distCfg.SetEndpoints(vec_eps, engine_conf.dist_endpoints(cur_index));
      distCfg.SetCommInitConfig(engine_conf.dist_cfg_file());

      config.SetDistConfig(distCfg);
      LOG(INFO) << "Create Distributed predictor! dist_carrier_id="
                << engine_conf.dist_carrier_id()
                << ", Ranks=" << engine_conf.dist_nranks()
                << ", current index of ranks=" << cur_index
                << ", current endpoint="
                << engine_conf.dist_endpoints(cur_index)
                << ", communicate init config file="
                << engine_conf.dist_cfg_file();
    }

    config.SwitchSpecifyInputNames(true);
    config.SetCpuMathLibraryNumThreads(engine_conf.cpu_math_thread_num());
    if (engine_conf.has_use_gpu() && engine_conf.use_gpu()) {
      // 2000MB GPU memory
      config.EnableUseGpu(engine_conf.gpu_memory_mb(), gpu_id);
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

    int local_min_subgraph_size = min_subgraph_size;
    if (engine_conf.has_min_subgraph_size()) {
      local_min_subgraph_size = engine_conf.min_subgraph_size();
    }

    if (engine_conf.has_use_trt() && engine_conf.use_trt()) {
      config.SwitchIrOptim(true);
      if (!engine_conf.has_use_gpu() || !engine_conf.use_gpu()) {
        config.EnableUseGpu(engine_conf.gpu_memory_mb(), gpu_id);
        if (engine_conf.has_gpu_multi_stream() &&
            engine_conf.gpu_multi_stream()) {
          config.EnableGpuMultiStream();
        }
      }
      config.EnableTensorRtEngine(engine_conf.trt_workspace_size(),
                                  max_batch,
                                  local_min_subgraph_size,
                                  precision_type,
                                  engine_conf.trt_use_static(),
                                  FLAGS_use_calib);
      std::map<std::string, std::vector<int>> min_input_shape;
      std::map<std::string, std::vector<int>> max_input_shape;
      std::map<std::string, std::vector<int>> optim_input_shape;
      if (engine_conf.min_input_shape_size() > 0) {
        for (auto& iter : engine_conf.min_input_shape()) {
          std::string key = iter.first;
          std::string value = iter.second;
          std::istringstream ss(value);
          std::string word;
          std::vector<int> arr;
          while (ss >> word) {
            arr.push_back(std::stoi(word));
          }
          min_input_shape[key] = arr;
        }
      }
      if (engine_conf.max_input_shape_size() > 0) {
        for (auto& iter : engine_conf.max_input_shape()) {
          std::string key = iter.first;
          std::string value = iter.second;
          std::istringstream ss(value);
          std::string word;
          std::vector<int> arr;
          while (ss >> word) {
            arr.push_back(std::stoi(word));
          }
          max_input_shape[key] = arr;
        }
      }
      if (engine_conf.opt_input_shape_size() > 0) {
        for (auto& iter : engine_conf.opt_input_shape()) {
          std::string key = iter.first;
          std::string value = iter.second;
          std::istringstream ss(value);
          std::string word;
          std::vector<int> arr;
          while (ss >> word) {
            arr.push_back(std::stoi(word));
          }
          optim_input_shape[key] = arr;
        }
      }
      config.SetTRTDynamicShapeInfo(
          min_input_shape, max_input_shape, optim_input_shape);
      LOG(INFO) << "create TensorRT predictor";
    }

    if (engine_conf.has_use_lite() && engine_conf.use_lite()) {
      config.EnableLiteEngine(precision_type, true);
      config.SwitchIrOptim(true);
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
      config.SetXpuDeviceId(gpu_id);
    }

    if (engine_conf.has_use_ascend_cl() && engine_conf.use_ascend_cl()) {
      if (engine_conf.has_use_lite() && engine_conf.use_lite()) {
        // for ascend 310
        FLAGS_nnadapter_device_names = "huawei_ascend_npu";
        FLAGS_nnadapter_context_properties =
            "HUAWEI_ASCEND_NPU_SELECTED_DEVICE_IDS=" + std::to_string(gpu_id);
        FLAGS_nnadapter_model_cache_dir = "";
        config.NNAdapter()
            .Enable()
            .SetDeviceNames({FLAGS_nnadapter_device_names})
            .SetContextProperties(FLAGS_nnadapter_context_properties)
            .SetModelCacheDir(FLAGS_nnadapter_model_cache_dir);
        LOG(INFO) << "Enable Lite NNAdapter for Ascend,"
                  << "nnadapter_device_names=" << FLAGS_nnadapter_device_names
                  << ",nnadapter_context_properties="
                  << FLAGS_nnadapter_context_properties
                  << ",nnadapter_model_cache_dir="
                  << FLAGS_nnadapter_model_cache_dir;
      } else {
        // for ascend 910
        config.EnableNpu(gpu_id);
      }
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

    LOG(INFO) << "paddle_engine params : enable_dist_model:"
              << engine_conf.enable_dist_model()
              << ", use_gpu: " << engine_conf.has_use_gpu()
              << ", gpu_id: " << gpu_id
              << ", use_gpu_multi_stream: " << engine_conf.gpu_multi_stream()
              << ", precision: " << FLAGS_precision
              << ", enable_ir_optimization: "
              << engine_conf.enable_ir_optimization()
              << ", use_trt: " << engine_conf.use_trt()
              << ", trt_max_batch: " << max_batch
              << ", trt_min_subgraph_size: " << min_subgraph_size
              << ", use_calib: " << FLAGS_use_calib
              << ", use_lite: " << engine_conf.use_lite()
              << ", use_ascend_cl: " << engine_conf.has_use_ascend_cl()
              << ", use_xpu: " << engine_conf.use_xpu()
              << ", enable_memory_optimization: "
              << engine_conf.enable_memory_optimization()
              << ", gpu_memory_mb: " << engine_conf.gpu_memory_mb()
              << ", cpu_math_thread_num: " << engine_conf.cpu_math_thread_num()
              << ", trt_workspace_size: " << engine_conf.trt_workspace_size()
              << ", trt_use_static: " << engine_conf.trt_use_static();

    VLOG(2) << "create paddle predictor sucess, path: " << model_path;
    return 0;
  }
};

}  // namespace inference
}  // namespace paddle_serving
}  // namespace baidu
