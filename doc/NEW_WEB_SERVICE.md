# How to develop a new Web service?


([简体中文](NEW_WEB_SERVICE_CN.md)|English)

This document will take Uci service as an example to introduce how to develop a new Web Service. You can check out the complete code [here](../python/examples/pipeline/simple_web_service/web_service.py).

## Op base class

In some services, a single model may not meet business needs, requiring multiple models to be concatenated or parallel to complete the entire service. We call a single model operation Op and provide a simple set of interfaces to implement the complex logic of Op concatenation or parallelism.

Data between Ops is passed as a dictionary, Op can be started as threads or process, and Op can be configured for the number of concurrencies, etc.

Typically, you need to inherit the Op base class and override its `init_op`,  `preprocess` and `postprocess` methods, which are implemented by default as follows:

```python
class Op(object):
  def init_op(self):
    pass
  def preprocess(self, input_dicts):
    # multiple previous Op
    if len(input_dicts) != 1:
      _LOGGER.critical(
        "Failed to run preprocess: this Op has multiple previous "
        "inputs. Please override this func.")
      os._exit(-1)
    (_, input_dict), = input_dicts.items()
    return input_dict
  def postprocess(self, input_dicts, fetch_dict):
    return fetch_dict
```

### init_op

This method is used to load user-defined resources such as dictionaries. A separator is loaded in the [UciOp](../python/examples/pipeline/simple_web_service/web_service.py).

**Note**: If Op is launched in threaded mode, different threads of the same Op execute `init_op` only once and share `init_op` loaded resources when Op is multi-concurrent.

### preprocess

This method is used to preprocess the data before model prediction. It has an `input_dicts` parameter, `input_dicts` is a dictionary, key is the `name` of the previous Op, and value is the data transferred from the corresponding previous op (the data is also in dictionary format).

The `preprocess` method needs to process the data into a ndarray dictionary (key is the feed variable name, and value is the corresponding ndarray value). Op will take the return value as the input of the model prediction and pass the output to the `postprocess` method.

**Note**: if Op does not have a model configuration file, the return value of `preprocess` will be directly passed to `postprocess`.

### postprocess

This method is used for data post-processing after model prediction. It has two parameters, `input_dicts` and `fetch_dict`.

Where the `input_dicts` parameter is consistent with the parameter in `preprocess` method, and `fetch_dict` is the output of the model prediction (key is the name of the fetch variable, and value is the corresponding ndarray value). Op will take the return value of `postprocess` as the input of subsequent Op `preprocess`.

**Note**: if Op does not have a model configuration file, `fetch_dict` will be the return value of `preprocess`.



Here is the op of the UCI example:

```python
class UciOp(Op):
    def init_op(self):
        self.separator = ","

    def preprocess(self, input_dicts):
        (_, input_dict), = input_dicts.items()
        x_value = input_dict["x"]
        if isinstance(x_value, (str, unicode)):
            input_dict["x"] = np.array(
                [float(x.strip()) for x in x_value.split(self.separator)])
        return input_dict

    def postprocess(self, input_dicts, fetch_dict):
        fetch_dict["price"] = str(fetch_dict["price"][0][0])
        return fetch_dict
```



## WebService base class

Paddle Serving implements the [WebService](https://github.com/PaddlePaddle/Serving/blob/develop/python/paddle_serving_server/web_service.py#L23) base class. You need to override its `get_pipeline_response` method to define the topological relationship between Ops. The default implementation is as follows:

```python
class WebService(object):
  def get_pipeline_response(self, read_op):
    return None
```

Where `read_op` serves as the entry point of the topology map of the whole service (that is, the first op defined by the user is followed by `read_op`).

For single Op service (single model), take Uci service as an example (there is only one Uci prediction model in the whole service):

```python
class UciService(WebService):
  def get_pipeline_response(self, read_op):
    uci_op = UciOp(name="uci", input_ops=[read_op])
    return uci_op
```

For multiple Op services (multiple models), take Ocr service as an example (the whole service is completed in series by Det model and Rec model):

```python
class OcrService(WebService):
  def get_pipeline_response(self, read_op):
    det_op = DetOp(name="det", input_ops=[read_op])
    rec_op = RecOp(name="rec", input_ops=[det_op])
    return rec_op
```



WebService objects need to load a yaml configuration file through the `prepare_pipeline_config` to configure each Op and the entire service. The simplest configuration file is as follows (Uci example):

```yaml
http_port: 18080
op:
    uci:
        local_service_conf:
            model_config: uci_housing_model # path
```

All field names of yaml file are as follows:

```yaml
rpc_port: 18080  # gRPC port
build_dag_each_worker: false  # Whether to use process server or not. The default is false
worker_num: 1  # gRPC thread pool size (the number of processes in the process version servicer). The default is 1
http_port: 0 # HTTP service port. Do not start HTTP service when the value is less or equals 0. The default value is 0.
dag:
    is_thread_op: true  # Whether to use the thread version of OP. The default is true
    client_type: brpc  # Use brpc or grpc client. The default is brpc
    retry: 1  # The number of times DAG executor retries after failure. The default value is 1, that is, no retrying
    use_profile: false  # Whether to print the log on the server side. The default is false
    tracer:
        interval_s: -1 # Monitoring time interval of Tracer (in seconds). Do not start monitoring when the value is less than 1. The default value is -1
op:
    <op_name>: # op name, corresponding to the one defined in the program
        concurrency: 1 # op concurrency number, the default is 1
        timeout: -1 # predict timeout in milliseconds. The default value is -1, that is, no timeout
        retry: 1 # timeout retransmissions. The default value is 1, that is, do not try again
        batch_size: 1 # If this field is set, Op will merge multiple request outputs into a single batch
        auto_batching_timeout: -1 # auto-batching timeout in milliseconds. The default value is -1, that is, no timeout
        local_service_conf:
            model_config: # the path of the corresponding model file. There is no default value(None). If this item is not configured, the model file will not be loaded.
            workdir: "" # working directory of corresponding model
            thread_num: 2 # the corresponding model is started with thread_num threads
            devices: "" # on which device does the model launched. You can specify the GPU card number(such as "0,1,2"), which is CPU by default
            mem_optim: true # mem optimization option, the default is true
            ir_optim: false # ir optimization option, the default is false
```

All fields of Op can be defined when Op is created in the program (which will override yaml fields).
