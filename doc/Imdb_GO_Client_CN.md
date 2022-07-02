# 如何在Paddle Serving使用Go Client

(简体中文|[English](./Imdb_GO_Client_EN.md))

本文档说明了如何将Go用作客户端语言。对于Paddle Serving中的Go客户端，提供了一个简单的客户端程序包https://github.com/PaddlePaddle/Serving/tree/develop/go/serving_client, 用户可以根据需要引用该程序包。这是一个基于IMDB数据集的情感分析任务的简单示例。

### 安装

我们假设您已经安装了Go 1.9.2或更高版本，并且安装了python 2.7版本

```shell
go get github.com/PaddlePaddle/Serving/go/serving_client
go get github.com/PaddlePaddle/Serving/go/proto
pip install paddle-serving-server -i https://pypi.tuna.tsinghua.edu.cn/simple
```
### 下载文本分类模型

```shell
wget https://paddle-serving.bj.bcebos.com/data/text_classification/imdb_serving_example.tar.gz
tar -xzf imdb_serving_example.tar.gz
```

### 启动服务器

```shell
python -m paddle_serving_server.serve --model ./serving_server_model/ --port 9292
```

### 客户端代码示例

``` go
// imdb_client.go
package main

import (
       "io"
       "fmt"
       "strings"
       "bufio"
       "strconv"
       "os"
       serving_client "github.com/PaddlePaddle/Serving/go/serving_client"
)

func main() {
     var config_file_path string
     config_file_path = os.Args[1]
     handle := serving_client.LoadModelConfig(config_file_path)
     handle = serving_client.Connect("127.0.0.1", "9292", handle)

     test_file_path := os.Args[2]
     fi, err := os.Open(test_file_path)
     if err != nil {
     	fmt.Print(err)
     }

     defer fi.Close()
     br := bufio.NewReader(fi)

     fetch := []string{"cost", "acc", "prediction"}     

     var result map[string][]float32

     for {
     	 line, err := br.ReadString('\n')
	 if err == io.EOF {
	       break
	 }

	 line = strings.Trim(line, "\n")

	 var words = []int64{}

	 s := strings.Split(line, " ")
	 value, err := strconv.Atoi(s[0])
	 var feed_int_map map[string][]int64
       
	 for _, v := range s[1:value + 1] {
	       int_v, _ := strconv.Atoi(v)
	       words = append(words, int64(int_v))
	 }

	 label, err := strconv.Atoi(s[len(s)-1])

	 if err != nil {
	       panic(err)
	 }

	 feed_int_map = map[string][]int64{}
	 feed_int_map["words"] = words
	 feed_int_map["label"] = []int64{int64(label)}
	 
	 result = serving_client.Predict(handle, feed_int_map, fetch)
	 fmt.Println(result["prediction"][1], "\t", int64(label))
    }
}
```

### 基于IMDB测试集的预测

```python
go run imdb_client.go serving_client_conf/serving_client_conf.stream.prototxt test.data> result
```

### 计算精度

```python
// acc.go
package main

import (
       "io"
       "os"
       "fmt"
       "bufio"
       "strings"
       "strconv"
)

func main () {
     score_file: = os.Args [1]
     fi, err: = os.Open (score_file)
     if err! = nil {
     fmt.Print (err)
     }

     defer fi.Close ()
     br: = bufio.NewReader (fi)
    
     total: = int (0)
     acc: = int (0)
     for {
     line, err: = br.ReadString ('\ n')
     if err == io.EOF {
        break
     }
    
     line = strings.Trim (line, "\ n")
     s: = strings.Split (line, "\ t")
     prob_str: = strings.Trim (s [0], "")
     label_str: = strings.Trim (s [1], "")
     prob, err: = strconv.ParseFloat (prob_str, 32)
     if err! = nil {
        panic (err)
     }
     label, err: = strconv.ParseFloat (label_str, 32)
     if err! = nil {
        panic (err)
     }
     if (prob-0.5) * (label-0.5)> 0 {
        acc ++
     }
     total ++
    }
    fmt.Println ("total num:", total)
    fmt.Println ("acc num:", acc)
    fmt.Println ("acc:", float32 (acc) / float32 (total))

}
```

```shell
go run acc.go result
total num: 25000
acc num: 22014
acc: 0.88056
```
