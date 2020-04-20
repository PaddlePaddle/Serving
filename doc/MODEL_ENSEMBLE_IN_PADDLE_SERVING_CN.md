# Paddle Serving中的集成预测

(简体中文|[English](MODEL_ENSEMBLE_IN_PADDLE_SERVING.md))

在一些场景中，可能使用多个相同输入的模型并行集成预测以获得更好的预测效果，Paddle Serving提供了这项功能。

下面将以文本分类任务为例，来展示Paddle Serving的集成预测功能（暂时还是串行预测，我们会尽快支持并行化）。

## 集成预测样例

该样例中（见下图），Server端在一项服务中并行预测相同输入的BOW和CNN模型，Client端获取两个模型的预测结果并进行后处理，得到最终的预测结果。

![simple example](model_ensemble_example.png)

需要注意的是，目前只支持在同一个服务中使用多个相同格式输入输出的模型。在该例子中，CNN模型和BOW模型的输入输出格式是相同的。

样例中用到的代码保存在`python/examples/imdb`路径下：

```shell
.
├── get_data.sh
├── imdb_reader.py
├── test_ensemble_client.py
└── test_ensemble_server.py
```

### 数据准备

通过下面命令获取预训练的CNN和BOW模型（您也可以直接运行`get_data.sh`脚本）：

```shell
wget --no-check-certificate https://fleet.bj.bcebos.com/text_classification_data.tar.gz
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/imdb-demo/imdb_model.tar.gz
tar -zxvf text_classification_data.tar.gz
tar -zxvf imdb_model.tar.gz
```

### 启动Server

通过下面的Python代码启动Server端（您也可以直接运行`test_ensemble_server.py`脚本）：

```python
from paddle_serving_server import OpMaker
from paddle_serving_server import OpGraphMaker
from paddle_serving_server import Server

op_maker = OpMaker()
read_op = op_maker.create('general_reader')
cnn_infer_op = op_maker.create(
    'general_infer', engine_name='cnn', inputs=[read_op])
bow_infer_op = op_maker.create(
    'general_infer', engine_name='bow', inputs=[read_op])
response_op = op_maker.create(
    'general_response', inputs=[cnn_infer_op, bow_infer_op])

op_graph_maker = OpGraphMaker()
op_graph_maker.add_op(read_op)
op_graph_maker.add_op(cnn_infer_op)
op_graph_maker.add_op(bow_infer_op)
op_graph_maker.add_op(response_op)

server = Server()
server.set_op_graph(op_graph_maker.get_op_graph())
model_config = {cnn_infer_op: 'imdb_cnn_model', bow_infer_op: 'imdb_bow_model'}
server.load_model_config(model_config)
server.prepare_server(workdir="work_dir1", port=9393, device="cpu")
server.run_server()
```

与普通预测服务不同的是，这里我们需要用DAG来描述Server端的运行逻辑。

在创建Op的时候需要指定当前Op的前继（在该例子中，`cnn_infer_op`与`bow_infer_op`的前继均是`read_op`，`response_op`的前继是`cnn_infer_op`和`bow_infer_op`），对于预测Op`infer_op`还需要定义预测引擎名称`engine_name`（也可以使用默认值，建议设置该值方便Client端获取预测结果）。

同时在配置模型路径时，需要以预测Op为key，对应的模型路径为value，创建模型配置字典，来告知Serving每个预测Op使用哪个模型。

### 启动Client

通过下面的Python代码运行Client端（您也可以直接运行`test_ensemble_client.py`脚本）：

```python
from paddle_serving_client import Client
from imdb_reader import IMDBDataset

client = Client()
# If you have more than one model, make sure that the input
# and output of more than one model are the same.
client.load_client_config('imdb_bow_client_conf/serving_client_conf.prototxt')
client.connect(["127.0.0.1:9393"])

# you can define any english sentence or dataset here
# This example reuses imdb reader in training, you
# can define your own data preprocessing easily.
imdb_dataset = IMDBDataset()
imdb_dataset.load_resource('imdb.vocab')

for i in range(3):
    line = 'i am very sad | 0'
    word_ids, label = imdb_dataset.get_words_and_label(line)
    feed = {"words": word_ids}
    fetch = ["acc", "cost", "prediction"]
    fetch_maps = client.predict(feed=feed, fetch=fetch)
    if len(fetch_maps) == 1:
        print("step: {}, res: {}".format(i, fetch_maps['prediction'][0][1]))
    else:
        for model, fetch_map in fetch_maps.items():
            print("step: {}, model: {}, res: {}".format(i, model, fetch_map[
                'prediction'][0][1]))
```

Client端与普通预测服务没有发生太大的变化。当使用多个模型预测时，预测服务将返回一个key为Server端定义的引擎名称`engine_name`，value为对应的模型预测结果的字典。

### 预期结果

```txt
step: 0, model: cnn, res: 0.560272455215
step: 0, model: bow, res: 0.633530199528
step: 1, model: cnn, res: 0.560272455215
step: 1, model: bow, res: 0.633530199528
step: 2, model: cnn, res: 0.560272455215
step: 2, model: bow, res: 0.633530199528
```
