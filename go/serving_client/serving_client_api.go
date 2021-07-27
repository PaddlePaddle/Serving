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
       pb "github.com/PaddlePaddle/Serving/go/proto"
       "github.com/golang/protobuf/proto"
)

type Tensor struct {
     Data   []byte `json:"data"`
     FloatData	   []float32 `json:"float_data"`
     IntData	   []int `json:"int_data"`
     Int64Data	   []int64 `json:"int64_data"`
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
     ProfileServer	bool `json:"profile_server"`
}

type Response struct {
     Insts    []FetchInst `json:"insts"`
     ProfileTime	  []int64 `json:"profile_time"`     
}

type Handle struct {
     Url    string
     Port   string
     FeedAliasNameMap	map[string]string
     FeedShapeMap	map[string][]int
     FeedNameMap   map[string]int
     FeedAliasNames	   []string
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
     log.Println("read protobuf succeed")
     handle := Handle{}
     handle.FeedNameMap = map[string]int{}
     handle.FeedAliasNameMap = map[string]string{}
     handle.FeedShapeMap = map[string][]int{}
     handle.FetchNameMap = map[string]int{}
     handle.FetchAliasNameMap = map[string]string{}
     handle.FeedAliasNames = []string{}

     for i, v := range general_model_config.FeedVar {
     	 handle.FeedNameMap[*v.Name] = i
	 tmp_array := []int{}
	 for _, vv := range v.Shape {
	     tmp_array = append(tmp_array, int(vv))
	 }
	 handle.FeedShapeMap[*v.Name] = tmp_array
	 handle.FeedAliasNameMap[*v.AliasName] = *v.Name
	 handle.FeedAliasNames = append(handle.FeedAliasNames, *v.AliasName)
     }

     for i, v := range general_model_config.FetchVar {
     	 handle.FetchNameMap[*v.Name] = i
	 handle.FetchAliasNameMap[*v.AliasName] = *v.Name
     }

     return handle
}

func Connect(url string, port string, handle Handle) Handle {
     handle.Url = url
     handle.Port = port
     return handle
}

func Predict(handle Handle, int_feed_map map[string][]int64, fetch []string) map[string][]float32 {
     contentType := "application/json;charset=utf-8"

     var tensor_array []Tensor
     var inst FeedInst
     tensor_array = []Tensor{}
     inst = FeedInst{}

     for i := 0; i < len(handle.FeedAliasNames); i++ {
     	 key_i := handle.FeedAliasNames[i]
	 var tmp Tensor
	 tmp.IntData = []int{}
	 tmp.Shape = []int{}
	 tmp.Int64Data = int_feed_map[key_i]
	 tmp.ElemType = 0
	 tmp.Shape = handle.FeedShapeMap[key_i]
	 tensor_array = append(tensor_array, tmp)
     }

     inst.TensorArray = tensor_array

     var profile_server bool
     profile_server = false

     req := &Request{
     	 Insts: []FeedInst{inst},
	 FetchVarNames: fetch,
	 ProfileServer: profile_server}

     b, err := json.Marshal(req)

     body := bytes.NewBuffer(b)

     var post_address bytes.Buffer
     post_address.WriteString("http://")
     post_address.WriteString(handle.Url)
     post_address.WriteString(":")
     post_address.WriteString(handle.Port)
     post_address.WriteString("/GeneralModelService/inference")

     resp, err := http.Post(post_address.String(), contentType, body)
     if err != nil {
     	log.Println("Post failed:", err)
     }

     defer resp.Body.Close()

     content, err := ioutil.ReadAll(resp.Body)
     if err != nil {
      	log.Println("Read failed:", err)
     }

     response_json := Response{}
     err = json.Unmarshal([]byte(content), &response_json)

     var result map[string][]float32
     result = map[string][]float32{}
     for i, v := range fetch {
     	 result[v] = response_json.Insts[0].TensorArray[i].FloatData
     }
     
     return result
}
