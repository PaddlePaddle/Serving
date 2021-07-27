# HTTP Inferface

Paddle Serving服务均可以通过HTTP接口访问，客户端只需按照Service定义的Request消息格式构造json字符串即可。客户端构造HTTP请求，将json格式数据以POST请求发给serving端，serving端**自动**按Service定义的Protobuf消息格式，将json数据转换成protobuf消息。

本文档介绍以python和PHP语言访问Serving的HTTP服务接口的用法。

## 1. 访问地址

访问Serving节点的HTTP服务与C++服务使用同一个端口（例如8010），访问URL规则为：

```
http://127.0.0.1:8010/ServiceName/inference
http://127.0.0.1:8010/ServiceName/debug
```

其中ServiceName应该与Serving的配置文件`conf/services.prototxt`中配置的一致，假如有如下2个service：

```protobuf
services {
  name: "BuiltinTestEchoService"
  workflows: "workflow3"
}

services {
  name: "TextClassificationService"
  workflows: "workflow6"
}
```

则访问上述2个Serving服务的HTTP URL分别为：

```
http://127.0.0.1:8010/BuiltinTestEchoService/inference
http://127.0.0.1:8010/BuiltinTestEchoService/debug

http://127.0.0.1:8010/TextClassificationService/inference
http://127.0.0.1:8010/TextClassificationService/debug
```

## 2. Python访问HTTP Serving

Python语言访问HTTP Serving，关键在于构造json格式的请求数据，可以通过以下步骤完成：

1) 按照Service定义的Request消息格式构造python object
2) `json.dump()` / `json.dumps()` 等函数将python object转换成json格式字符串

以TextClassificationService为例，关键代码如下：

```python
# Connect to server
conn = httplib.HTTPConnection("127.0.0.1", 8010)

# samples是一个list，其中每个元素是一个ids字典：
# samples[0] = [190, 1, 70, 382, 914, 5146, 190...]
for i in range(0, len(samples) - BATCH_SIZE, BATCH_SIZE):
    # 构建批量预测数据
    batch = samples[i: i + BATCH_SIZE]
    ids = []
    for x in batch:
        ids.append({"ids" : x})
    ids = {"instances": ids}

    # python object转成json
    request_json = json.dumps(ids)

    # 请求HTTP服务，打印response
    try:
        conn.request('POST', "/TextClassificationService/inference", request_json, {"Content-Type": "application/json"})
        response = conn.getresponse()
        print response.read()
    except httplib.HTTPException as e:
        print e.reason
```

完整示例请参考[text_classification.py](https://github.com/PaddlePaddle/Serving/blob/develop/tools/cpp_examples/demo-client/python/text_classification.py)

## 3. PHP访问HTTP Serving

PHP语言构造json格式字符串的步骤如下：

1) 按照Service定义的Request消息格式，构造PHP array
2) `json_encode()`函数将PHP array转换成json字符串

以TextCLassificationService为例，关键代码如下：

```PHP
function http_post(&$ch, $data) {
    // array to json string
    $data_string = json_encode($data);

    // post data 封装
    curl_setopt($ch, CURLOPT_POSTFIELDS, $data_string);

    // set header
    curl_setopt($ch,
            CURLOPT_HTTPHEADER,
            array(
                'Content-Length: ' . strlen($data_string)
            )
    );

    // 执行
    $result = curl_exec($ch);
    return $result;
}

$ch = &http_connect('http://127.0.0.1:8010/TextClassificationService/inference');

$count = 0;

# $samples是一个2层array，其中每个元素是一个如下array：
# $samples[0] = array(
#                   "ids" => array(
#                                 [0] => int(190),
#                                 [1] => int(1),
#                                 [2] => int(70),
#                                 [3] => int(382),
#                                 [4] => int(914),
#                                 [5] => int(5146),
#                                 [6] => int(190)...)
#                    )

for ($i = 0; $i < count($samples) - BATCH_SIZE; $i += BATCH_SIZE) {
    $instances = array_slice($samples, $i, BATCH_SIZE);
    echo http_post($ch, array("instances" => $instances)) . "\n";
}

curl_close($ch);
```

完整代码请参考[text_classification.php](https://github.com/PaddlePaddle/Serving/blob/develop/tools/cpp_examples/demo-client/php/text_classification.php)
