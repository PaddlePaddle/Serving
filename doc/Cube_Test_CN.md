## 如果获得稀疏参数索引Cube所需的模型输入

### 背景知识

推荐系统需要大规模稀疏参数索引来帮助分布式部署，可在`Serving/examples/C++/PaddleRec/criteo_ctr_with_cube`或是[PaddleRec](https://github.com/paddlepaddle/paddlerec)了解推荐模型。

稀疏参数索引的模型格式是SequenceFile，源自Hadoop生态的键值对格式文件。

为了方便调试，我们给出了从特定格式的可读文本文件到SequenceFile格式文件的转换工具，以及SequenceFile格式文件与可阅读文字的转换。

用户在调试Cube服务功能时，可以自定义KV对生成SequenceFile格式文件来进行调试。
用户在验证Cube的配送正确性时，可以转换SequenceFile格式文件至可读文字来进行比对验证。

### 预备知识

- 需要会编译Paddle Serving，参见[编译文档](./Compile_EN.md)

### 用法

在编译结束后的安装文件，可以得到 seq_reader 和 kv_to_seqfile.py。

#### 生成SequenceFile

在`output/tool/`下，修改`output/tool/source/file.txt`，该文件每一行对应一个键值对，用冒号`:`区分key和value部分。

例如：
```
1676869128226002114:48241       37064           91      -539    114     51      -122    269     229     -134    -282
1657749292782759014:167 40              98      27      117     10      -29     15      74      67      -54
```
执行
```
python kv_to_seqfile.py
```
即可生成`data`文件夹，我们看下它的结构

```
.
├── 20210805095422
│   └── base
│       └── feature
└── donefile
    └── base.txt
```
其中`20210805095422/base/feature` 就是SequenceFile格式文件，donefile保存在`donefile/base.txt`。

#### 查看SequenceFile

我们使用`seq_reader`工具来解读SequenceFile格式文件。
```
./seq_reader 20210805095422/base/feature 10 # 阅读开头的10个KV对
./seq_reader 20210805095422/base/feature # 阅读所有KV对
```

结果
```
key: 1676869128226002114 , value: 343832343109333730363409093931092D35333909313134093531092D3132320932363909323239092D313334092D323832
key: 1657749292782759014 , value: 3136370934300909393809323709313137093130092D3239093135093734093637092D3534
```

其中value 我们目前都以16进制的形式打印。
