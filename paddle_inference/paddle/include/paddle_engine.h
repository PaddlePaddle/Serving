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

// data interface
class PaddleInfencceEngine {
 public:
  virtual ~FluidFamilyCore() {}
  virtual std::vector<std::string> GetInputNames() {
    return _core->GetInputNames();
  }

  virtual std::unique_ptr<Tensor> GetInputHandle(const std::string& name) {
    return _core->GetInputHandle(name);
  }

  virtual std::vector<std::string> GetOutputNames() {
    return _core->GetOutputNames();
  }

  virtual std::unique_ptr<Tensor> GetOutputHandle(const std::string& name) {
    return _core->GetOutputHandle(name);
  }

  virtual bool Run() {
    if (!_core->Run()) {
      LOG(ERROR) << "Failed call Run with paddle predictor";
      return false;
    }
    return true;
  }

  virtual int create(const predictor::InferEngineCreationParams& params) = 0;

  virtual int clone(void* origin_core) {
    if (origin_core == NULL) {
      LOG(ERROR) << "origin paddle Predictor is null.";
      return -1;
    }
    Predictor* p_predictor = (Predictor*)origin_core;
    _core = p_predictor->Clone();
    if (_core.get() == NULL) {
      LOG(ERROR) << "fail to clone paddle predictor: " << origin_core;
      return -1;
    }
    return 0;
  }

  virtual void* get() { return _core.get(); }

 protected:
  std::shared_ptr<Predictor> _core;
};

// infer interface
class FluidCpuAnalysisCore : public FluidFamilyCore {
 public:
  int create(const predictor::InferEngineCreationParams& params) {
    std::string data_path = params.get_path();
    if (access(data_path.c_str(), F_OK) == -1) {
      LOG(ERROR) << "create paddle predictor failed, path not exits: "
                 << data_path;
      return -1;
    }

    Config config;
    config.SetParamsFile(data_path + "/__params__");
    config.SetProgFile(data_path + "/__model__");
    config.DisableGpu();
    config.SetCpuMathLibraryNumThreads(1);

    if (params.enable_memory_optimization()) {
      config.EnableMemoryOptim();
    }

    config.SwitchSpecifyInputNames(true);
    AutoLock lock(GlobalPaddleCreateMutex::instance());
    _core = CreatePredictor(config);
    if (NULL == _core.get()) {
      LOG(ERROR) << "create paddle predictor failed, path: " << data_path;
      return -1;
    }

    VLOG(2) << "create paddle predictor sucess, path: " << data_path;
    return 0;
  }
};

class FluidCpuAnalysisDirCore : public FluidFamilyCore {
 public:
  int create(const predictor::InferEngineCreationParams& params) {
    std::string data_path = params.get_path();
    if (access(data_path.c_str(), F_OK) == -1) {
      LOG(ERROR) << "create paddle predictor failed, path not exits: "
                 << data_path;
      return -1;
    }

    Config config;
    config.SetModel(data_path);
    config.DisableGpu();
    config.SwitchSpecifyInputNames(true);
    config.SetCpuMathLibraryNumThreads(1);

    if (params.enable_memory_optimization()) {
      config.EnableMemoryOptim();
    }

    if (params.enable_ir_optimization()) {
      config.SwitchIrOptim(true);
    } else {
      config.SwitchIrOptim(false);
    }

    AutoLock lock(GlobalPaddleCreateMutex::instance());
    _core = CreatePredictor(config);
    if (NULL == _core.get()) {
      LOG(ERROR) << "create paddle predictor failed, path: " << data_path;
      return -1;
    }

    VLOG(2) << "create paddle predictor sucess, path: " << data_path;
    return 0;
  }
};

class FluidCpuAnalysisEncryptCore : public FluidFamilyCore {
 public:
  void ReadBinaryFile(const std::string& filename, std::string* contents) {
    std::ifstream fin(filename, std::ios::in | std::ios::binary);
    fin.seekg(0, std::ios::end);
    contents->clear();
    contents->resize(fin.tellg());
    fin.seekg(0, std::ios::beg);
    fin.read(&(contents->at(0)), contents->size());
    fin.close();
  }

  int create(const predictor::InferEngineCreationParams& params) {
    std::string data_path = params.get_path();
    if (access(data_path.c_str(), F_OK) == -1) {
      LOG(ERROR) << "create paddle predictor failed, path note exits: "
                 << data_path;
      return -1;
    }

    std::string model_buffer, params_buffer, key_buffer;
    ReadBinaryFile(data_path + "encrypt_model", &model_buffer);
    ReadBinaryFile(data_path + "encrypt_params", &params_buffer);
    ReadBinaryFile(data_path + "key", &key_buffer);

    VLOG(2) << "prepare for encryption model";

    auto cipher = paddle::MakeCipher("");
    std::string real_model_buffer = cipher->Decrypt(model_buffer, key_buffer);
    std::string real_params_buffer = cipher->Decrypt(params_buffer, key_buffer);

    Config analysis_config;
    // paddle::AnalysisConfig analysis_config;
    analysis_config.SetModelBuffer(&real_model_buffer[0],
                                   real_model_buffer.size(),
                                   &real_params_buffer[0],
                                   real_params_buffer.size());
    analysis_config.DisableGpu();
    analysis_config.SetCpuMathLibraryNumThreads(1);
    if (params.enable_memory_optimization()) {
      analysis_config.EnableMemoryOptim();
    }
    analysis_config.SwitchSpecifyInputNames(true);
    AutoLock lock(GlobalPaddleCreateMutex::instance());
    VLOG(2) << "decrypt model file sucess";
    _core = CreatePredictor(analysis_config);
    if (NULL == _core.get()) {
      LOG(ERROR) << "create paddle predictor failed, path: " << data_path;
      return -1;
    }
    VLOG(2) << "create paddle predictor sucess, path: " << data_path;
    return 0;
  }
};

}  // namespace fluid_cpu
}  // namespace paddle_serving
}  // namespace baidu
