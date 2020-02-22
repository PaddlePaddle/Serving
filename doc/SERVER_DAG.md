# Computation Graph On Server

This document shows the concept of computation graph on server. How to define computation graph with PaddleServing built-in operators. Examples for some sequential execution logics are shown as well.

## Computation Graph on Server

Deep neural nets often have some preprocessing steps on input data, and postprocessing steps on model inference scores. Since deep learning frameworks are now very flexible, it is possible to do preprocessing and postprocessing outside the training computation graph. If we want to do input data preprocessing and inference result postprocess on server side, we have to add the corresponding computation logics on server. Moreover, if a user wants to do inference with the same inputs on more than one model, the best way is to do the inference concurrently on server side given only one client request so that we can save some network computation overhead. For the above two reasons, it is naturally to think of a Directed Acyclic Graph(DAG) as the main computation method for server inference. One example of DAG is as follows. [one possible DAG on Serving](server_dag.png)



## How to define Node

PaddleServing has some predefined Computation Node in the framework. A very commonly used Computation Graph is the simple reader-inference-response mode that can cover most of the single model inference scenarios. A example graph and the corresponding DAG defination code is as follows.

``` python
import paddle_serving_server as serving
op_maker = serving.OpMaker()
read_op = op_maker.create('general_reader')
general_infer_op = op_maker.create('general_infer')
general_response_op = op_maker.create('general_response')

op_seq_maker = serving.OpSeqMaker()
op_seq_maker.add_op(read_op)
op_seq_maker.add_op(general_infer_op)
op_seq_maker.add_op(general_response_op)
```

Since the code will be commonly used and users do not have to change the code, PaddleServing releases a easy-to-use launching command for service startup. An example is as follows: 

``` python
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292
```

## More Examples

If a user has sparse features as inputs, and the model will do embedding lookup for each feature, we can do distributed embedding lookup operation which is not in the Paddle training computation graph. An example is as follows:

``` python
import paddle_serving_server as serving
op_maker = serving.OpMaker()
read_op = op_maker.create('general_reader')
dist_kv_op = op_maker.create('general_dist_kv')
general_infer_op = op_maker.create('general_infer')
general_response_op = op_maker.create('general_response')

op_seq_maker = serving.OpSeqMaker()
op_seq_maker.add_op(read_op)
op_seq_maker.add_op(dist_kv_op)
op_seq_maker.add_op(general_infer_op)
op_seq_maker.add_op(general_response_op)
```







