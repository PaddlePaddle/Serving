"""
----------------------- Env info ------------------------
cuda_version: 10.1
cudnn_version: 7.6.5
trt_version: 6.0.15 
python_version: 3.7 
gcc_version: 8.2  # 7.5, 5.4
paddle_version: v2.0.1 #tag or commit id
cpu: 6148 gold # i7, 6148, 6271
gpu: t4 # p4, v100, 1080
xpu: xxx # xxx
api: cpp # python, cpp, go, c
owner: lipeihan01 # 测试负责人
----------------------- Model info ----------------------
model_name: ch_ppocr_mobile_v1.1_cls_infer
model_type: static # dy2static, onnx
model_sorce: PaddleOCR # PaddleDetection, PaddleSeg
model_url: http://paddlexxxx # download url
----------------------- Data info -----------------------
batch_size: 1
num_of_samples: 1000
input_shape: 3,640,640
----------------------- Conf info -----------------------
runtime_device: gpu
ir_optim: true
enable_memory_optim: true
enable_tensorrt: true
precision: int8    # fp32, fp16, int8
enable_mkldnn: true
cpu_math_library_num_threads: 4
----------------------- Acc info ------------------------
acc1:
acc5:
----------------------- Perf info -----------------------
average_latency(ms): 0.962421, QPS: 1039.05
process_latency(ms): xxxx # 前后处理时间
process_name: clas_benchmark, cpu_rss(MB): 2113.92578125, vms(MB): 13119.50390625, shared(MB): 432.0859375, dirty(MB): 0.0, cpu_usage(%): 8.07324178110695
gpu_id: 0, total(MB): 7611.9375, free(MB): 7336.4375, used(MB): 1505.5, gpu_utilization_rate(%): 100.0, gpu_mem_utilization_rate(%): 65.0

"""

import sys

fstr = ""
fstr += "----------------------- Env info ------------------------\n"
fstr += "cuda_version: {}\n".format(cuda_version)
fstr += "cudnn_version: {}\n".format(cudnn_version)
fstr += "trt_version: {}\n".format(trt_version)
fstr += "gcc_version: {}\n".format(gcc_version)
fstr += "paddle_version: {}\n".format(paddle_version)
fstr += "cpu: {}\n".format(cpu)
fstr += "gpu: {}\n".format(gpu)
fstr += "xpu: {}\n".format(xpu)
fstr += "api: {}\n".format(api)
fstr += "owner: {}\n".format(owner)

fstr += "----------------------- Model info ----------------------"
