#   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

package main

import (
       "bytes"
       "encoding/json"
       "io/ioutil"
       "log"
       "net/http"
       "encoding/binary"
       "fmt"
       "general_model_config"
)

type Tensor struct {
     IntData	   []int64 `json:"int_data"`
     ElemType	int `json:"elem_type"`
     Shape	[]int `json:"shape"`
}

type FeedInst struct {
     TensorArray     []Tensor `json:"tensor_array"`
}

type FetchInst struct {
     TensorArray      []Tensor `json:"tensor_array"`
}

type Request struct {
     Insts   []FeedInst `json:"insts"`
     FetchVarNames	[]string `json:"fetch_var_names"`
}

type Response struct {
     Insts    []FetchInst `json:"insts"`
}

type Handle struct {
     Url    string
     Port   int
     FeedAliasNameMap	Map[string]string
     FeedShapeMap	Map[string][]int
     FeedNameMap   Map[string]int
     FetchNameMap  Map[string]int
     FetchAliasNameMap	Map[string]string
}

func LoadModelConfig(config string) Handle {
     in, err := ioutil.ReadFile(config)
     if err != nil {
     	log.Fatalln("Failed to read general model: ", err)
     }
     general_model_config := &pb.GeneralConfig{}
     if err := proto.Unmarshal(in, general_model_config); err != nil {
     	log.Fatalln("Failed to parse GeneralConfig: ", err)
     }
     handle := Handle{}
     for i, v in range general_model_config.FeedVar {
     	 handle.FeedNameMap[v.Name] = i
	 handle.FeedShapeMap[v.Name] = v
	 handle.FeedAliasNameMap[v.AliasName] = v.Name
     }

     for i, v in range general_model_config.FetchVar {
     	 handle.FetchNameMap[v.Name] = i
	 handle.FetchAliasNameMap[v.AliasName] = v.Name
     }

     return handle
}

func Connect(url string, port int, handle Handle) Handle {
     handle.Url = url
     handle.Port = port
     return handle
}

func Predict(handle Handle, int_feed_map Map[string]int{}, fetch []string) {
     contentType := "application/json;charset=utf-8"

     var tensor_array []Tensor{}
     var inst FeedInst
     for k, v := range int_feed_map {
     	 var tmp Tensor
	 tmp.Data = int_feed_map[k]
	 tmp.ElemType = 0
	 tmp.Shape = handle.FeedNameShape[k]
	 tensor_array = append(tensor_array, tmp)
     }

     req := &Request{
     	 Insts: []FeedInst{inst},
	 FetchVarNames: []string{fetch}}

     b, err := json.Marshal(req)
     fmt.Println(string(b))

     body := bytes.NewBuffer(b)

     resp, err := http.Post(url, contentType, body)
     if err != nil {
     	log.Println("Post failed:", err)
	return
     }

     defer resp.Body.Close()

     content, err := ioutil.ReadAll(resp.Body)
     if err != nil {
     	log.Println("Read failed:", err)
	return
     }

     log.Println("content:", string(content))
}
