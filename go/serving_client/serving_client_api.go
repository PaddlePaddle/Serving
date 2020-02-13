//   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package serving_client

import (
       "bytes"
       "encoding/json"
       "io/ioutil"
       "log"
       "net/http"
       "fmt"
       pb "general_model"
       "github.com/golang/protobuf/proto"
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
     FeedAliasNameMap	map[string]string
     FeedShapeMap	map[string][]int
     FeedNameMap   map[string]int
     FetchNameMap  map[string]int
     FetchAliasNameMap	map[string]string
}

func LoadModelConfig(config string) Handle {
     in, err := ioutil.ReadFile(config)
     if err != nil {
     	log.Fatalln("Failed to read general model: ", err)
     }
     general_model_config := &pb.GeneralModelConfig{}
     if err := proto.Unmarshal(in, general_model_config); err != nil {
     	log.Fatalln("Failed to parse GeneralModelConfig: ", err)
     }
     handle := Handle{}
     for i, v := range general_model_config.FeedVar {
     	 handle.FeedNameMap[*v.Name] = i
	 // handle.FeedShapeMap[*v.Name] := []int
	 tmp_array := []int{}
	 for _, vv := range v.Shape {
	     tmp_array = append(tmp_array, int(vv))
	 }
	 handle.FeedShapeMap[*v.Name] = tmp_array
	 handle.FeedAliasNameMap[*v.AliasName] = *v.Name
     }

     for i, v := range general_model_config.FetchVar {
     	 handle.FetchNameMap[*v.Name] = i
	 handle.FetchAliasNameMap[*v.AliasName] = *v.Name
     }

     return handle
}

func Connect(url string, port int, handle Handle) Handle {
     handle.Url = url
     handle.Port = port
     return handle
}

func Predict(handle Handle, int_feed_map map[string][]int64, fetch []string) map[string][]float32 {
     contentType := "application/json;charset=utf-8"

     var tensor_array []Tensor
     var inst FeedInst
     for k, _ := range int_feed_map {
     	 var tmp Tensor
	 tmp.IntData = int_feed_map[k]
	 tmp.ElemType = 0
	 tmp.Shape = handle.FeedShapeMap[k]
	 tensor_array = append(tensor_array, tmp)
     }

     req := &Request{
     	 Insts: []FeedInst{inst},
	 FetchVarNames: fetch}

     b, err := json.Marshal(req)
     fmt.Println(string(b))

     body := bytes.NewBuffer(b)

     resp, err := http.Post(handle.Url, contentType, body)
     if err != nil {
     	log.Println("Post failed:", err)
     }

     defer resp.Body.Close()

     content, err := ioutil.ReadAll(resp.Body)
     if err != nil {
     	log.Println("Read failed:", err)
     }

     log.Println("content:", string(content))

     var result map[string][]float32
     return result
}
