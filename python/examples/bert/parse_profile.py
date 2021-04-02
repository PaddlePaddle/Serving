import sys 
import os
## general info
cuda_version = "10.1"
cudnn_version = "7"
trt_version = "6"
python_version = "3.6"
gcc_version = "8.2"
paddle_version = "2.0.0"
cpu = "Xeon 6148"
gpu = "P4"
xpu = ""
api = ""
owner = ""
## model info
model_name = "bert_seq128"
model_type = "static"
model_source = "paddlehub"
model_url = ""
## data info
batch_size = "1"
num_of_samples = "1"
input_shape = "none"
## conf info
runtime_device = "cpu"
ir_optim = "true"
enable_memory_optim = "true"
enable_tensorrt = "false"
precision = "fp32"
enable_mkldnn = "false"
cpu_math_library_num_threads = "1"
## acc info 
acc1 = ""
acc5 = ""
## perf info
average_latency, QPS = "", ""
process_latency = ""
cpu_rss, vms, shared, dirty, cpu_usage = "", "", "", "", ""
gpu_id, total, free, used, gpu_utilization_rate, gpu_mem_utilization_rate = "","","","","", ""

class LogHandler(object):
    def __init__(self):
        self.fstr = ""
    
    def print(self):
        print(self.fstr)

    def dump(self):
        with open("inference_profile.log",'w') as fout:
            fout.write(self.fstr)

    def append(self, new_str):
        self.fstr += new_str + "\n"

fh = LogHandler()

fh.append("cuda_version: {}".format(cuda_version))
fh.append("cudnn_version: {}".format(cudnn_version))
fh.append("trt_version: {} ".format(trt_version))
fh.append("python_version: {}".format(python_version))
fh.append("gcc_version: {}".format(gcc_version))
fh.append("paddle_version: {}".format(paddle_version))
fh.append("cpu: {}".format(cpu))
fh.append("gpu: {}".format(gpu)) # p4, v100, 1080
fh.append("xpu: {}".format(xpu)) 
fh.append("api: {}".format(api))
fh.append("owner: {}".format(owner))
fh.append("----------------------- Model info ----------------------")
fh.append("model_name: {}".format(model_name))
fh.append("model_type: {}".format(model_type))
fh.append("model_source: {}".format(model_source))
fh.append("model_url: {}".format(model_url))
fh.append("----------------------- Data info -----------------------")
fh.append("batch_size: {}".format(batch_size))
fh.append("num_of_samples: {}".format(num_of_samples))
fh.append("input_shape: {}".format(input_shape))
fh.append("----------------------- Conf info -----------------------")
fh.append("runtime_device: {}".format(runtime_device))
fh.append("ir_optim: {}".format(ir_optim))
fh.append("enable_memory_optim: {}".format(enable_memory_optim))
fh.append("enable_tensorrt: {}".format(enable_tensorrt))
fh.append("precision: {}".format(precision))    # fp32, fp16, int8
fh.append("enable_mkldnn: {}".format(enable_mkldnn))
fh.append("cpu_math_library_num_threads: {}".format(cpu_math_library_num_threads))
fh.append("----------------------- Acc info ------------------------")
fh.append("acc1:".format(acc1))
fh.append("acc5:".format(acc5))
fh.append("----------------------- Perf info -----------------------")
fh.append("average_latency(ms): {}, QPS: {}".format(average_latency, QPS))
fh.append("process_latency(ms): {}".format(process_latency))
fh.append("process_name: clas_benchmark, cpu_rss(MB): {}, vms(MB): {}, shared(MB): {}, dirty(MB): {}, cpu_usage(%): {}".format(cpu_rss, vms, shared, dirty, cpu_usage))
fh.append("gpu_id: {}, total(MB): {}, free(MB): {}, used(MB): {}, gpu_utilization_rate(%): {}, gpu_mem_utilization_rate(%): {}".format(gpu_id, total, free, used, gpu_utilization_rate, gpu_mem_utilization_rate))

fh.dump()
