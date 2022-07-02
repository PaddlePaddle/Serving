// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
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

package dict

type (
	DictVersionInfo struct {
		DictName   string         `json:"dict_name"`
		Version    int            `json:"version"`
		Depend     int            `json:"depend"`
		Id         int            `json:"id"`
		Key        int            `json:"key"`
		Mode       string         `json:"mode"`
		Input      string         `json:"input"`
		Output     string         `json:"output"`
		Status     DictStatus     `json:"status"`
		StatusStr  DictStatusStr  `json:"status_str"`
		FinishTime int            `json:"finish_time"`
		CreateTime int            `json:"create_time"`
		MetaInfos  map[int]string `json:"meta_infos"`
	}
	DonefileInfo struct {
		Id    string `json:"id"`
		Key   string `json:"key"`
		Input string `json:"input"`
	}
	DictShardMetaInfo struct {
		Name string `json:"name"`
		Version int    `json:"version"`
		Depend  int    `json:"depend"`
		Shard   int    `json:"shard"`
		Split   int    `json:"split"`
		Meta    string `json:"meta"`
	}

	MetaInfo struct {
		IndexTotalCount string   `json:"index_total_count"`
		IndexLenList    []string `json:"index_len_list"`
		DataLenList     []string `json:"data_len_list"`
	}
)

