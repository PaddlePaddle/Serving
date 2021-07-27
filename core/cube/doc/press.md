# cube压测文档

参考[大规模稀疏参数服务Cube的部署和使用](https://github.com/PaddlePaddle/Serving/blob/master/doc/DEPLOY.md#2-大规模稀疏参数服务cube的部署和使用)文档进行cube的部署。

压测工具链接：

https://paddle-serving.bj.bcebos.com/data/cube/cube-press.tar.gz

将压缩包解压，cube-press目录下包含了单机场景和分布式场景下client端压测所使用的脚本、可执行文件、配置文件、样例数据以及数据生成脚本。

其中，keys为client要读取用来查询的key值文件，feature为cube要加载的key-value文件，本次测试中使用的数据，key的范围为0~999999。

## 单机场景

在单个物理机部署cube服务，使用genernate_input.py脚本生成测数据，执行test.sh脚本启动cube client端向cube server发送请求。

genernate_input.py脚本接受1个参数，示例：

```bash
python genernate_input.py 1
```

参数表示生成的数据每一行含有多少个key，即test.sh执行的查询操作中的batch_size。


test.sh脚本接受3个参数，示例：

```bash
sh test.sh 1 127.0.0.1:8027 100000
```

第一个参数为并发数，第二个参数为cube server的ip与端口，第三个参数为qps。

输出：

脚本会进行9次压测，每次发送10次请求，每次请求耗时1秒，每次压测会打印出平均延时以及不同分位数的延时。

**注意事项：**

cube压测对于机器的网卡要求较高，高QPS的情况下单个client可能无法承受，可以采用两个或多个client，将查询请求进行平均。

如果执行test.sh出现问题需要停止，可以执行kill_rpc_press.sh

## 分布式场景

编译paddle serving完成后，分布式压测工具的客户端路径为 build/core/cube/cube-api/cube-cli，对应的源代码为core/cube/cube-api/src/cube_cli.cpp

在多台机器上部署cube服务，使用client_cli进行性能测试。

**注意事项：**

cube服务部署时的分片数和副本数会对性能造成影响，相同数据的条件下，分片数和副本数越多，性能越好，实际提升程度与数据相关。

使用方法：

```shell
./cube-cli --batch 500 --keys keys --dict dict --thread_num 1
```

接受的参数：

--batch指定每次请求的batch size。

--keys指定查询用的文件，文件中每一行为1个key。

--dict指定要查询的cube词典名。

--thread_num指定client端线程数

输出：

每个线程的查询的平均时间、最大时间、最小时间

进程中所有线程的查询的平均时间的平均值、最大值、最小值

进程中所有线程的总请求数、QPS
