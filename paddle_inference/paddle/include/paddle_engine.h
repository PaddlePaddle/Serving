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

#include <pthread.h>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "core/configure/include/configure_parser.h"
#include "core/configure/inferencer_configure.pb.h"
#include "core/predictor/framework/infer.h"
#include "paddle_inference_api.h"  // NOLINT

namespace baidu {
namespace paddle_serving {
namespace inference {

using paddle_infer::Config;
using paddle_infer::Predictor;
using paddle_infer::Tensor;
using paddle_infer::CreatePredictor;

const static int max_batch = 32;
const static int min_subgraph_size = 3;
// Engine Base
class PaddleEngineBase {
 public:
  virtual ~PaddleEngineBase() {}
  virtual std::vector<std::string> GetInputNames() {
    return _predictor -> GetInputNames();
  }

  virtual std::unique_ptr<Tensor> GetInputHandle(const std::string& name) {
    return _predictor -> GetInputHandle(name);
  }

  virtual std::vector<std::string> GetOutputNames() {
    return _predictor -> GetOutputNames();
  }

  virtual std::unique_ptr<Tensor> GetOutputHandle(const std::string& name) {
    return _predictor -> GetOutputHandle(name);
  }

  virtual bool Run() {
    if (!_predictor -> Run()) {
      LOG(ERROR) << "Failed call Run with paddle predictor";
      return false;
    }
    return true;
  }

  virtual int create(const configure::EngineDesc& conf) = 0;

  virtual int clone(void* predictor) {
    if (predictor == NULL) {
      LOG(ERROR) << "origin paddle Predictor is null.";
      return -1;
    }
    Predictor*  prep = static_cast<Predictor*>(predictor);
    _predictor = prep -> Clone();
    if (_predictor.get() == NULL) {
      LOG(ERROR) << "fail to clone paddle predictor: " << predictor;
      return -1;
    }
    return 0;
  }

  virtual void* get() { return _predictor.get(); }

 protected:
  std::shared_ptr<Predictor> _predictor;
};

// Paddle Inference Engine
class PaddleInferenceEngine : public PaddleEngineBase {
 public:
  int create(const configure::EngineDesc& engine_conf) {
    std::string model_path = engine_conf.model_dir();
    if (access(model_path.c_str(), F_OK) == -1) {
      LOG(ERROR) << "create paddle predictor failed, path not exits: "
                 << model_path;
      return -1;
    }

    Config config;
    // todo, auto config(zhangjun)
    if(engine_conf.has_combined_model()) {
      if(!engine_conf.combined_model()) {
        config.SetModel(model_path)
      } else {
        config.SetParamsFile(model_path + "/__params__");
        config.SetProgFile(model_path + "/__model__");
      }
    } else {
      config.SetParamsFile(model_path + "/__params__");
      config.SetProgFile(model_path + "/__model__");
    }
    
    config.SwitchSpecifyInputNames(true);
    config.SetCpuMathLibraryNumThreads(1);
    if (engine_conf.has_use_gpu() && engine_conf.use_gpu()) {
      // 2000MB GPU memory
      config.EnableUseGpu(2000, FLAGS_gpuid);
    }
  
    if (engine_conf.has_use_trt() && engine_conf.use_trt()) {
      config.EnableTensorRtEngine(1 << 20,
                                  max_batch,
                                  min_subgraph_size,
                                  Config::Precision::kFloat32,
                                  false,
                                  false);
      LOG(INFO) << "create TensorRT predictor";
    }

    if (engine_conf.has_lite() && engine_conf.use_lite()) {
      config.EnableLiteEngine(PrecisionType::kFloat32, true);
    }

    if (engine_conf.has_xpu() && engine_conf.use_xpu()) {
      // 2 MB l3 cache
      config.EnableXpu(2 * 1024 * 1024);
    }
    if (engine_conf.has_enable_ir_optimization() && !engine_conf.enable_ir_optimization()) {
      config.SwitchIrOptim(false);
    } else {
      config.SwitchIrOptim(true);
    }

    if (engine_conf.has_enable_memory_optimization() && engine_conf.enable_memory_optimization()) {
      config.EnableMemoryOptim();
    }
    
    if (false) {
      // todo, encrypt model
      //analysis_config.SetModelBuffer();
    }

    AutoLock lock(GlobalPaddleCreateMutex::instance());
    _predictor = CreatePredictor(config);
    if (NULL == _predictor.get()) {
      LOG(ERROR) << "create paddle predictor failed, path: " << data_path;
      return -1;
    }

    VLOG(2) << "create paddle predictor sucess, path: " << data_path;
    return 0;
  }
};

}  // namespace inference
}  // namespace paddle_serving
}  // namespace baidu
