# 如何开启 TensorRT 并配置动态 shape
(简体中文|[English](./TensorRT_Dynamic_Shape_EN.md))

## 概览

TensorRT是一个高性能的深度学习推理（Inference）优化器，可以为深度学习应用提供低延迟、高吞吐率的部署推理。
以下将分别从 Pipeline Serving 和 C++ Serving 介绍 Tensorrt 开启方式以及配置动态 shape(Dynamic Shape)。

## Paddle Inference Dynamic Shape Api
```
  void SetTRTDynamicShapeInfo(
      std::map<std::string, std::vector<int>> min_input_shape,
      std::map<std::string, std::vector<int>> max_input_shape,
      std::map<std::string, std::vector<int>> optim_input_shape,
      bool disable_trt_plugin_fp16 = false);
```
具体API说明请参考[C++](https://paddleinference.paddlepaddle.org.cn/api_reference/cxx_api_doc/Config/GPUConfig.html#tensorrt)/[Python](https://paddleinference.paddlepaddle.org.cn/api_reference/python_api_doc/Config/GPUConfig.html#tensorrt)

## C++ Serving 

**一. C++ Serving Tensorrt 开启方式**

在 C++ Serving 启动命令加上`--use_trt`

```
python -m paddle_serving_server.serve \
--model serving_server \
--thread 2 --port 9000 \
--gpu_ids 0 \
--use_trt \
--precision FP16
```

**二. C++ Serving 设置动态 shape**

在`**/paddle_inference/paddle/include/paddle_engine.h` 修改如下代码

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


## Pipeline Serving

**一. Pipeline Serving Tensorrt 开启方式**

在示例目录下的 config.yml 文件, 修改`device_type: 2`, 配置 GPU 使用的核心 `devices: "0,1,2,3"`
>> **注意**: Tensorrt 需要配合 GPU 使用

**二. Pipeline Serving 设置动态 shape**

在示例目录下的 web_service.py, 在每个 op 下可以通过 `def set_dynamic_shape_info(self):` 添加动态 shape 相关的配置

示例如下
```
def set_dynamic_shape_info(self):
    min_input_shape = {
        "x": [1, 3, 50, 50],
        "conv2d_182.tmp_0": [1, 1, 20, 20],
        "nearest_interp_v2_2.tmp_0": [1, 1, 20, 20],
        "nearest_interp_v2_3.tmp_0": [1, 1, 20, 20],
        "nearest_interp_v2_4.tmp_0": [1, 1, 20, 20],
        "nearest_interp_v2_5.tmp_0": [1, 1, 20, 20]
    }
    max_input_shape = {
        "x": [1, 3, 1536, 1536],
        "conv2d_182.tmp_0": [20, 200, 960, 960],
        "nearest_interp_v2_2.tmp_0": [20, 200, 960, 960],
        "nearest_interp_v2_3.tmp_0": [20, 200, 960, 960],
        "nearest_interp_v2_4.tmp_0": [20, 200, 960, 960],
        "nearest_interp_v2_5.tmp_0": [20, 200, 960, 960],
    }
    opt_input_shape = {
        "x": [1, 3, 960, 960],
        "conv2d_182.tmp_0": [3, 96, 240, 240],
        "nearest_interp_v2_2.tmp_0": [3, 96, 240, 240],
        "nearest_interp_v2_3.tmp_0": [3, 24, 240, 240],
        "nearest_interp_v2_4.tmp_0": [3, 24, 240, 240],
        "nearest_interp_v2_5.tmp_0": [3, 24, 240, 240],
    }
    self.dynamic_shape_info = {
        "min_input_shape": min_input_shape,
        "max_input_shape": max_input_shape,
        "opt_input_shape": opt_input_shape,
    }
                
```
具体可以参考[Pipeline OCR](../examples/Pipeline/PaddleOCR/ocr/)
>> **注意**: 由于不同的模型具有不同的动态 shape 配置，因此不存在通用的动态 shape 配置方法。当运行 Pipeline Serving 
>> 出现报错信息时，应该使用[netron](https://netron.app/) 加载模型，查看各个 op 的输入输出 shape。之后，结合报错信息，在 web_service.py 
>> 添加相应的动态 shape 配置代码。
