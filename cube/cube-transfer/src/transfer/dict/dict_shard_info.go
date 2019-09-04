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

import (
	"strconv"
)

type DictShardInfo struct {
	Name           string `json:"name"`
	Version        string `json:"version"`
	Depend         string `json:"depend"`
	Id             string `json:"id"`
	Key            string `json:"key"`
	Shard          int    `json:"shard"`
	Mode           string `json:"mode"`
	DictMode       string `json:"dict_mode,omitempty"`
	Source         string `json:"data_source"`
	Service        string `json:"service,omitempty"`
	DeltaInfo      string `json:"delta_info,omitempty"`
	BuildedTime    int    `json:"builded_time,omitempty"`
	BuildedTimeStr string `json:"build_finish_time,omitempty"`
	CreateTime     int    `json:"create_time,omitempty"`
	IsActive       bool   `json:"is_active,omitempty"`
}

func GetDictShardScaler(shard int, dictVersionInfo DictVersionInfo, downloadMode string, transferAddr string, wgetPort string)(info DictShardInfo){
	info.Name = dictVersionInfo.DictName
	info.Version = strconv.Itoa(dictVersionInfo.Version)
	info.Depend = strconv.Itoa(dictVersionInfo.Depend)
	info.Id = strconv.Itoa(dictVersionInfo.Id)
	info.Key = strconv.Itoa(dictVersionInfo.Key)
	info.Mode = dictVersionInfo.Mode
	info.Shard = shard
	info.Source = GetFileHead(downloadMode, transferAddr, wgetPort) + dictVersionInfo.Output+ "/" + info.Version + "/" + info.Name + "_part" + strconv.Itoa(shard) + ".tar"
	return
}


func GetFileHead(downloadMode string, transferAddr string, wgetPort string) string {
	if downloadMode == "http" {
		return HTTP_HEADER + transferAddr + ":" + wgetPort
	} else if downloadMode == "ftp" {
		return FTP_HEADER + transferAddr + ":" + wgetPort
	} else {
		return ""
	}
}