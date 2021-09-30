# cube python api说明文档
参考[大规模稀疏参数服务Cube的部署和使用](https://github.com/PaddlePaddle/Serving/blob/master/doc/DEPLOY.md#2-大规模稀疏参数服务cube的部署和使用)文档进行cube的部署。
使用python api，可替代上述文档中第3节预测服务的部署、使用

## 配置说明
conf/cube.conf 以json格式，设置各个分片cube server的ip以及port，shard与分片数一致，示例：
```bash
[{
    "dict_name": "test",
    "shard": 2,
    "nodes": [{
        "ip": "127.0.0.1",
        "port": 8731
    },{
        "ip": "127.0.0.1",
        "port": 8730
    }]
}]
```

## 数据格式
```bash
{"keys": [0,1,2,3,4,5,6,7]}
{"keys": [1]}
```
支持批量查询，每次查询一行

## 使用
```bash
cd ./python-api
python3 demo.py conf/cube.conf test input.json result.json
```