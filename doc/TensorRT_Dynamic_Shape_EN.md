# How to configure TensorRT dynamic shape
([简体中文](./TensorRT_Dynamic_Shape_CN.md)|English)

## Overview

After enabling TensorRT `--use_trt` in Pipeline/C++. Regarding how to configure the dynamic shape, the following will give examples of Pipeline Serving and C++ Serving respectively

The following is the dynamic shape api
```
  void SetTRTDynamicShapeInfo(
      std::map<std::string, std::vector<int>> min_input_shape,
      std::map<std::string, std::vector<int>> max_input_shape,
      std::map<std::string, std::vector<int>> optim_input_shape,
      bool disable_trt_plugin_fp16 = false);
```
For detail, please refer to API doc [C++](https://paddleinference.paddlepaddle.org.cn/api_reference/cxx_api_doc/Config/GPUConfig.html#tensorrt)/[Python](https://paddleinference.paddlepaddle.org.cn/api_reference/python_api_doc/Config/GPUConfig.html#tensorrt)

### C++ Serving
Modify the following code in `**/paddle_inference/paddle/include/paddle_engine.h`

```
    if (engine_conf.has_use_trt() && engine_conf.use_trt()) {
      config.SwitchIrOptim(true);
      if (!engine_conf.has_use_gpu() || !engine_conf.use_gpu()) {
        config.EnableUseGpu(50, gpu_id);
        if (engine_conf.has_gpu_multi_stream() &&
            engine_conf.gpu_multi_stream()) {
          config.EnableGpuMultiStream();
        }
      }
      config.EnableTensorRtEngine((1 << 30) + (1 << 29),
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
        min_input_shape["_generated_var_64"] = {1, 1, 768};
        min_input_shape["fc_0.tmp_0"] = {1, 1, 768};
        min_input_shape["_generated_var_87"] = {1, 1, 768};
        min_input_shape["tmp_175"] = {1, 1, 768};
        min_input_shape["c_allreduce_sum_0.tmp_0"] = {1,1, 12288};
        min_input_shape["embedding_1.tmp_0"] = {1, 1, 12288};


        max_input_shape["stack_0.tmp_0"] = {bsz, 16, max_seq_len, max_seq_len};
        max_input_shape["stack_1.tmp_0"] = {bsz, 2, max_seq_len, max_seq_len};
        max_input_shape["input_mask"] = {bsz, max_seq_len, max_seq_len};
        max_input_shape["_generated_var_64"] = {bsz, max_seq_len, 768};
        max_input_shape["fc_0.tmp_0"] = {bsz, max_seq_len, 768};
        max_input_shape["_generated_var_87"] = {bsz, max_seq_len, 768};
        max_input_shape["tmp_175"] = {bsz, max_seq_len, 768};
        max_input_shape["c_allreduce_sum_0.tmp_0"] = {bsz,max_seq_len, 12288};
        max_input_shape["embedding_1.tmp_0"] = {bsz, max_seq_len, 12288};

        int g1 = 0;
        int g2 = 0;
        int t1 = 0;
        int t2 = 0;
        std::string var_name = "_generated_var_";
        std::string tmp_name = "tmp_";
        for (int i = 0; i < 44; ++i) {
          if (i > 32) {
            hidden_size = 768;
            g1 = 2*i-1;
            g2 = 2*i;
            t1 = 4*i-1;
            t2 = 4*i;
            min_input_shape[var_name+std::to_string(g1)] = {1, 1, hidden_size};
            min_input_shape[var_name+std::to_string(g2)] = {1, 1, hidden_size};
            min_input_shape[tmp_name+std::to_string(t1)] = {1, 1, hidden_size};
            min_input_shape[tmp_name+std::to_string(t2)] = {1, 1, hidden_size};
            max_input_shape[var_name+std::to_string(g1)] = {bsz, max_seq_len, hidden_size};
            max_input_shape[var_name+std::to_string(g2)] = {bsz, max_seq_len, hidden_size};
            max_input_shape[tmp_name+std::to_string(t1)] = {bsz, max_seq_len, hidden_size};
            max_input_shape[tmp_name+std::to_string(t2)] = {bsz, max_seq_len, hidden_size};
          }
          if (i <32) {
            hidden_size = 12288;
            g1 = 2*i;
            g2 = 2*i+1;
            t1 = 4*i;
            t2 = 4*i+3;
            min_input_shape[var_name+std::to_string(g1)] = {1, 1, hidden_size};
            min_input_shape[var_name+std::to_string(g2)] = {1, 1, hidden_size};
            min_input_shape[tmp_name+std::to_string(t1)] = {1, 1, hidden_size};
            min_input_shape[tmp_name+std::to_string(t2)] = {1, 1, hidden_size};
            max_input_shape[var_name+std::to_string(g1)] = {bsz, max_seq_len, hidden_size};
            max_input_shape[var_name+std::to_string(g2)] = {bsz, max_seq_len, hidden_size};
            max_input_shape[tmp_name+std::to_string(t1)] = {bsz, max_seq_len, hidden_size};
            max_input_shape[tmp_name+std::to_string(t2)] = {bsz, max_seq_len, hidden_size};
          }
        }
        optim_input_shape = max_input_shape;
        config.SetTRTDynamicShapeInfo(
        min_input_shape, max_input_shape, optim_input_shape);
      }
      LOG(INFO) << "create TensorRT predictor";
    }
```


### Pipeline Serving

Modify the following code in `**/python/paddle_serving_app/local_predict.py`

```
if use_trt:
    config.enable_tensorrt_engine(
        precision_mode=precision_type,
        workspace_size=1 << 20,
        max_batch_size=32,
        min_subgraph_size=3,
        use_static=False,
        use_calib_mode=False)
    head_number = 12

    names = [
        "placeholder_0", "placeholder_1", "placeholder_2", "stack_0.tmp_0"
    ]
    min_input_shape = [1, 1, 1]
    max_input_shape = [100, 128, 1]
    opt_input_shape = [10, 60, 1]

    config.set_trt_dynamic_shape_info(
        {
            names[0]: min_input_shape,
            names[1]: min_input_shape,
            names[2]: min_input_shape,
            names[3]: [1, head_number, 1, 1]
        }, {
            names[0]: max_input_shape,
            names[1]: max_input_shape,
            names[2]: max_input_shape,
            names[3]: [100, head_number, 128, 128]
        }, {
            names[0]: opt_input_shape,
            names[1]: opt_input_shape,
            names[2]: opt_input_shape,
            names[3]: [10, head_number, 60, 60]
        })
                
```