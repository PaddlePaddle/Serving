#pragma once

#include <pthread.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include "framework/infer.h"
#include "paddle/fluid/inference/paddle_inference_api.h"

namespace baidu {
namespace paddle_serving {
namespace fluid_cpu {

class AutoLock {
public:
    AutoLock(pthread_mutex_t& mutex) : _mut(mutex){
        pthread_mutex_lock(&mutex);
    }

    ~AutoLock() {
        pthread_mutex_unlock(&_mut);
    }

private:
    pthread_mutex_t& _mut;
};

class GlobalPaddleCreateMutex {
public:
    pthread_mutex_t& mutex() {
        return _mut;
    }

    static pthread_mutex_t& instance() {
        static GlobalPaddleCreateMutex gmutex;
        return gmutex.mutex();
    }

private:
    GlobalPaddleCreateMutex() {
        pthread_mutex_init(&_mut, NULL);
    }

    pthread_mutex_t _mut;
};

class GlobalSigmoidCreateMutex {
public:
    pthread_mutex_t& mutex() {
        return _mut;
    }
    static pthread_mutex_t& instance() {
    static GlobalSigmoidCreateMutex gmutex;
        return gmutex.mutex();
    }
private:
    GlobalSigmoidCreateMutex() {
        pthread_mutex_init(&_mut, NULL);
    }

    pthread_mutex_t _mut;
};

// data interface
class FluidFamilyCore {
public:

    virtual ~FluidFamilyCore() {}
    virtual bool Run(const void* in_data, void* out_data) {
        if (!_core->Run(*(std::vector<paddle::PaddleTensor>*)in_data, 
                    (std::vector<paddle::PaddleTensor>*)out_data)) {
            LOG(FATAL) << "Failed call Run with paddle predictor";
            return false;
        }

        return true;
    }

    virtual int create(const std::string& data_path) = 0;

    virtual int clone(void* origin_core) {
        if (origin_core == NULL) {
            LOG(FATAL) << "origin paddle Predictor is null.";
            return -1;
        }
        paddle::PaddlePredictor* p_predictor = (paddle::PaddlePredictor*)origin_core;
        _core = p_predictor->Clone();
        if (_core.get() == NULL) {
            LOG(FATAL) << "fail to clone paddle predictor: " << origin_core;
            return -1;
        }
        return 0;
    }

    virtual void* get() {
        return _core.get();
    }    

protected:
    std::unique_ptr<paddle::PaddlePredictor> _core;
};

// infer interface
class FluidCpuAnalysisCore : public FluidFamilyCore {
public:
    int create(const std::string& data_path) {
        if (access(data_path.c_str(), F_OK) == -1) {
            LOG(FATAL) << "create paddle predictor failed, path not exits: "
                << data_path;
            return -1;
        }

        paddle::contrib::AnalysisConfig analysis_config;
        analysis_config.param_file = data_path + "/__params__";
        analysis_config.prog_file = data_path + "/__model__";
        analysis_config.use_gpu = false;
        analysis_config.device = 0;
        analysis_config.specify_input_name = true;
        AutoLock lock(GlobalPaddleCreateMutex::instance());
        _core = paddle::CreatePaddlePredictor<
            paddle::contrib::AnalysisConfig>(analysis_config);
        if (NULL == _core.get()) {
            LOG(FATAL) << "create paddle predictor failed, path: "
                << data_path;
            return -1;
        }

        LOG(WARNING) << "create paddle predictor sucess, path: "<< data_path;
        return 0; 
    }
};

class FluidCpuNativeCore : public FluidFamilyCore {
public:
    int create(const std::string& data_path) {
        if (access(data_path.c_str(), F_OK) == -1) {
            LOG(FATAL) << "create paddle predictor failed, path not exits: "
                << data_path;
            return -1;
        }

        paddle::NativeConfig native_config;
        native_config.param_file = data_path + "/__params__";
        native_config.prog_file = data_path + "/__model__";
        native_config.use_gpu = false;
        native_config.device = 0;
        AutoLock lock(GlobalPaddleCreateMutex::instance());
        _core = paddle::CreatePaddlePredictor<
            paddle::NativeConfig, paddle::PaddleEngineKind::kNative>(native_config);
        if (NULL == _core.get()) {
            LOG(FATAL) << "create paddle predictor failed, path: "
                << data_path;
            return -1;
        }

        LOG(WARNING) << "create paddle predictor sucess, path: "<< data_path;
        return 0; 
    }
};

class FluidCpuAnalysisDirCore : public FluidFamilyCore {
public:
    int create(const std::string& data_path) {
        if (access(data_path.c_str(), F_OK) == -1) {
            LOG(FATAL) << "create paddle predictor failed, path not exits: "
                << data_path;
            return -1;
        }

        paddle::contrib::AnalysisConfig analysis_config;
        analysis_config.model_dir = data_path;
        analysis_config.use_gpu = false;
        analysis_config.device = 0;
        analysis_config.specify_input_name = true;
        AutoLock lock(GlobalPaddleCreateMutex::instance());
        _core = paddle::CreatePaddlePredictor<
            paddle::contrib::AnalysisConfig>(analysis_config);
        if (NULL == _core.get()) {
            LOG(FATAL) << "create paddle predictor failed, path: "
                << data_path;
            return -1;
        }

        LOG(WARNING) << "create paddle predictor sucess, path: "<< data_path;
        return 0; 
    }

};

class FluidCpuNativeDirCore : public FluidFamilyCore {
public:
    int create(const std::string& data_path) {
        if (access(data_path.c_str(), F_OK) == -1) {
            LOG(FATAL) << "create paddle predictor failed, path not exits: "
                << data_path;
            return -1;
        }

        paddle::NativeConfig native_config;
        native_config.model_dir = data_path;
        native_config.use_gpu = false;
        native_config.device = 0;
        AutoLock lock(GlobalPaddleCreateMutex::instance());
        _core = paddle::CreatePaddlePredictor<
            paddle::NativeConfig, paddle::PaddleEngineKind::kNative>(native_config);
        if (NULL == _core.get()) {
            LOG(FATAL) << "create paddle predictor failed, path: "
                << data_path;
            return -1;
        }

        LOG(WARNING) << "create paddle predictor sucess, path: "<< data_path;
        return 0; 
    }

};

class Parameter {
public:
    Parameter() : _row(0), _col(0), _params(NULL) {}
    ~Parameter() {
        LOG(INFO) << "before destroy Parameter, file_name[" << _file_name << "]";
        destroy();
    }

    int init(int row, int col, const char* file_name) {
        destroy();
        _file_name = file_name;
        _row = row;
        _col = col;
        _params = (float*)malloc(_row * _col * sizeof(float));
        if (_params == NULL) {
            LOG(FATAL) << "Load " << _file_name << " malloc error.";
            return -1;
        }
        LOG(WARNING) << "Load parameter file[" << _file_name << "] success.";
        return 0;
    }

    void destroy() {
        _row = 0;
        _col = 0;
        if (_params != NULL) {
            free(_params);
            _params = NULL;
        }
    }

    int load() {
        if (_params == NULL || _row <= 0 || _col <= 0) {
            LOG(FATAL) << "load parameter error [not inited].";
            return -1;
        }

        FILE* fs = fopen(_file_name.c_str(), "rb");
        if (fs == NULL) {
            LOG(FATAL) << "load " << _file_name << " fopen error.";
            return -1;
        }
        static const uint32_t MODEL_FILE_HEAD_LEN = 16;
        char head[MODEL_FILE_HEAD_LEN] = {0};
        if (fread(head, 1, MODEL_FILE_HEAD_LEN, fs) != MODEL_FILE_HEAD_LEN) {
            destroy();
            LOG(FATAL) << "Load " << _file_name << " read head error.";
            if (fs != NULL) {
                fclose(fs);
                fs = NULL;
            }
            return -1;
        }

        uint32_t matrix_size = _row * _col;
        if (matrix_size == fread(_params, sizeof(float), matrix_size, fs)) {
            if (fs != NULL) {
                fclose(fs);
                fs = NULL;
            }
            LOG(INFO) << "load " << _file_name << " read ok.";
            return 0;
        } else {
            LOG(FATAL) << "load " << _file_name << " read error.";
            destroy();
            if (fs != NULL) {
                fclose(fs);
                fs = NULL;
            }
            return -1;
        }
        return 0;
    }

public:
    std::string _file_name;
    int _row;
    int _col;
    float* _params;
};

class SigmoidModel {
public:
    ~SigmoidModel() {
    }
    int load(const char* sigmoid_w_file, const char* sigmoid_b_file,
            float exp_max, float exp_min) {
        AutoLock lock(GlobalSigmoidCreateMutex::instance());
        if (0 != _sigmoid_w.init(2, 1, sigmoid_w_file) || 0 != _sigmoid_w.load()) {
            LOG(FATAL) << "load params sigmoid_w failed.";
            return -1;
        }
        LOG(WARNING) << "load sigmoid_w [" << _sigmoid_w._params[0]
                << "] [" << _sigmoid_w._params[1] << "].";
        if (0 != _sigmoid_b.init(2, 1, sigmoid_b_file) || 0 != _sigmoid_b.load()) {
            LOG(FATAL) << "load params sigmoid_b failed.";
            return -1;
        }
        LOG(WARNING) << "load sigmoid_b [" << _sigmoid_b._params[0]
                << "] [" << _sigmoid_b._params[1] << "].";
        _exp_max_input = exp_max;
        _exp_min_input = exp_min;
        return 0;
    }

    int softmax(float x, double& o) {
        float _y0 = x * _sigmoid_w._params[0] + _sigmoid_b._params[0];
        float _y1 = x * _sigmoid_w._params[1] + _sigmoid_b._params[1];
        _y0 = (_y0 > _exp_max_input) ? _exp_max_input
                : ((_y0 < _exp_min_input) ? _exp_min_input : _y0);
        _y1 = (_y1 > _exp_max_input) ? _exp_max_input
                : ((_y1 < _exp_min_input) ? _exp_min_input : _y1);
        o = 1.0f / (1.0f + exp(_y0 - _y1));
        return 0;
    }
public:
    Parameter _sigmoid_w;
    Parameter _sigmoid_b;
    float _exp_max_input;
    float _exp_min_input;
};

class SigmoidFluidModel {
public:
    int softmax(float x, double& o) {
        return  _sigmoid_core->softmax(x, o);
    }

    std::unique_ptr<SigmoidFluidModel> Clone() {
        std::unique_ptr<SigmoidFluidModel> clone_model;
        clone_model.reset(new SigmoidFluidModel()); 
        clone_model->_sigmoid_core = _sigmoid_core;
        clone_model->_fluid_core = _fluid_core->Clone();
        return std::move(clone_model);
    }

public:
    std::unique_ptr<paddle::PaddlePredictor> _fluid_core;
    std::shared_ptr<SigmoidModel> _sigmoid_core;
};

class FluidCpuWithSigmoidCore : public FluidFamilyCore {
public:
    virtual  ~FluidCpuWithSigmoidCore() {
    }
public:
    int create(const std::string& model_path) {
        size_t pos = model_path.find_last_of("/\\");
        std::string conf_path = model_path.substr(0, pos);
        std::string conf_file = model_path.substr(pos);
        comcfg::Configure conf;
        if (conf.load(conf_path.c_str(), conf_file.c_str()) != 0) {
            LOG(FATAL) << "failed load model path: " << model_path;
            return -1;
        }

        _core.reset(new SigmoidFluidModel);
    
        std::string fluid_model_data_path = conf["dnn_model_path"].to_cstr();
        int ret = load_fluid_model(fluid_model_data_path);
        if (ret < 0) {
            LOG(FATAL) << "fail to load fluid model.";
            return -1;
        }
        const char* sigmoid_w_file = conf["sigmoid_w_file"].to_cstr();
        const char* sigmoid_b_file = conf["sigmoid_b_file"].to_cstr();
        float exp_max = conf["exp_max_input"].to_float();
        float exp_min = conf["exp_min_input"].to_float();
        _core->_sigmoid_core.reset(new SigmoidModel);
        LOG(INFO) << "create sigmoid core[" << _core->_sigmoid_core.get()
                << "], use count[" << _core->_sigmoid_core.use_count() << "].";
        ret = _core->_sigmoid_core->load(sigmoid_w_file, sigmoid_b_file, exp_max, exp_min);
        if (ret < 0) {
            LOG(FATAL) << "fail to load sigmoid model.";
            return -1;
        }
        return 0;
    }

    virtual bool Run(const void* in_data, void* out_data) {
        if (!_core->_fluid_core->Run(*(std::vector<paddle::PaddleTensor>*)in_data, 
                    (std::vector<paddle::PaddleTensor>*)out_data)) {
            LOG(FATAL) << "Failed call Run with paddle predictor";
            return false;
        }

        return true;
    }

    virtual int clone(SigmoidFluidModel* origin_core) {
        if (origin_core == NULL) {
            LOG(FATAL) << "origin paddle Predictor is null.";
            return -1;
        }
        _core = origin_core->Clone();
        if (_core.get() == NULL) {
            LOG(FATAL) << "fail to clone paddle predictor: " << origin_core;
            return -1;
        }
        LOG(INFO) << "clone sigmoid core[" << _core->_sigmoid_core.get()
                << "] use count[" << _core->_sigmoid_core.use_count() << "].";
        return 0;
    }
    
    virtual SigmoidFluidModel* get() {
        return _core.get();
    }

    virtual int load_fluid_model(const std::string& data_path) = 0;

    int softmax(float x, double& o) {
        return _core->_sigmoid_core->softmax(x, o);
    }

protected:
    std::unique_ptr<SigmoidFluidModel> _core;
};

class FluidCpuNativeDirWithSigmoidCore : public FluidCpuWithSigmoidCore {
public:
    int load_fluid_model(const std::string& data_path) {
        if (access(data_path.c_str(), F_OK) == -1) {
            LOG(FATAL) << "create paddle predictor failed, path not exits: "
                << data_path;
            return -1;
        }

        paddle::NativeConfig native_config;
        native_config.model_dir = data_path;
        native_config.use_gpu = false;
        native_config.device = 0;
        AutoLock lock(GlobalPaddleCreateMutex::instance());
        _core->_fluid_core = paddle::CreatePaddlePredictor<
            paddle::NativeConfig, paddle::PaddleEngineKind::kNative>(native_config);
        if (NULL == _core.get()) {
            LOG(FATAL) << "create paddle predictor failed, path: "
                << data_path;
            return -1;
        }

        LOG(WARNING) << "create paddle predictor sucess, path: "<< data_path;
        return 0; 
    }

};

class FluidCpuAnalysisDirWithSigmoidCore : public FluidCpuWithSigmoidCore {
public:
    int load_fluid_model(const std::string& data_path) {
        if (access(data_path.c_str(), F_OK) == -1) {
            LOG(FATAL) << "create paddle predictor failed, path not exits: "
                << data_path;
            return -1;
        }

        paddle::contrib::AnalysisConfig analysis_config;
        analysis_config.model_dir = data_path;
        analysis_config.use_gpu = false;
        analysis_config.device = 0;
        analysis_config.specify_input_name = true;
        AutoLock lock(GlobalPaddleCreateMutex::instance());
        _core->_fluid_core = paddle::CreatePaddlePredictor<
            paddle::contrib::AnalysisConfig>(analysis_config);
        if (NULL == _core.get()) {
            LOG(FATAL) << "create paddle predictor failed, path: "
                << data_path;
            return -1;
        }

        LOG(WARNING) << "create paddle predictor sucess, path: "<< data_path;
        return 0; 
    }
};

} // namespace fluid_cpu
} // namespace paddle_serving
} // namespace baidu
