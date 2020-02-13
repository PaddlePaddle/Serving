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

package main

import (
       "io"
       "strings"
       "bufio"
       "strconv"
       "fmt"
       "os"
       "serving_client"
)

func main() {
     var config_file_path string
     config_file_path = os.Args[1]
     handle := serving_client.LoadModelConfig(config_file_path)
     serving_client.Connect("127.0.0.1", 9292, handle)

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

	 var words = []int64{}

	 s := strings.Split(line, " ")
	 value, err := strconv.Atoi(s[0])
	 var feed_int_map map[string][]int64

	 for _, v := range s[1:value + 1] {
	     int_v, _ := strconv.Atoi(v)
	     words = append(words, int64(int_v))
	 }

	 label, _ := strconv.Atoi(s[len(s)-1])

	 feed_int_map["words"] = words
	 feed_int_map["label"] = []int64{int64(label)}
	 
	 result = serving_client.Predict(handle,
	 	 feed_int_map, fetch)
	 fmt.Println(result["prediction"][0])
     }
}