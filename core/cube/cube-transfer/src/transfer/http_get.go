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

package transfer

import (
	"encoding/json"
	"errors"
	"fmt"
	"strconv"
	"transfer/dict"
)

func initGetHandlers() {
	getHandler = map[string]handlerFunc{
		"/dict/info":                 GetDictInfo,
		"/instance/status":           GetInstanceStatus,
		"/dict/scaler":               GetDictScaler,
		"/dict/meta_info":            GetDictShardMetaInfo,
		"/dict/deploy/history":       GetDictDeployHistory,
	}
}
func GetDictInfo(subpath string, m map[string]string) (string, string, int, error) {
	b, err := json.Marshal(Dict)
	if err != nil {
		return quote(""), "", STATUS_FAIL, fmt.Errorf("json marshal failed, %v", err)
	}

	return string(b), "", STATUS_OK, nil
}

func GetInstanceStatus(subpath string, m map[string]string) (string, string, int, error) {
	b, err := json.Marshal(Dict.Instances)
	if err != nil {
		return quote(""), "", STATUS_FAIL, fmt.Errorf("json marshal failed, %v", err)
	}

	return string(b), "", STATUS_OK, nil
}

func GetDictScaler(subpath string, m map[string]string) (string, string, int, error) {
	var (
		shard int
		err   error
		infos []dict.DictShardInfo
	)
	shardStr, ok := m["shard"]
	if !ok {
		shard = 0
	} else {
		shard, err = strconv.Atoi(shardStr)
		if err != nil {
			return quote(""), "", STATUS_FAIL, errors.New("invalid arg: shard should be int")
		}
	}

	for _, version := range Dict.CurrentVersionInfo {
		info := dict.GetDictShardScaler(shard, version,  Dict.DownloadMode, TransferAddr, Dict.WgetPort)
		infos = append(infos, info)
	}
	if Dict.WaitVersionInfo.Status > dict.Dict_Status_Deploying {
		info := dict.GetDictShardScaler(shard, Dict.WaitVersionInfo,  Dict.DownloadMode, TransferAddr, Dict.WgetPort)
		infos = append(infos, info)
	}

	b, err := json.Marshal(infos)
	if err != nil {
		return quote(""), "", STATUS_FAIL, fmt.Errorf("json marshal failed, %v", err)
	}

	return quote(string(b)), "", STATUS_OK, nil
}

func GetDictShardMetaInfo(subpath string, m map[string]string) (string, string, int, error) {
	name, ok := m["name"]
	if !ok {
		return quote("failed"), "", STATUS_FAIL, errors.New("need arg: name")
	}
	version, err := strconv.Atoi(m["version"])
	if err != nil {
		return quote("failed"), "", STATUS_FAIL, errors.New("need arg: version, version should be int")
	}
	depend, err := strconv.Atoi(m["depend"])
	if err != nil {
		return quote("failed"), "", STATUS_FAIL, errors.New("need arg: depend, depend should be int")
	}
	shard, err := strconv.Atoi(m["shard"])
	if err != nil {
		return quote("failed"), "", STATUS_FAIL, errors.New("need arg: shard, shard should be int")
	}
	split := 1
	split_str, ok := m["split"]
	if ok {
		split, err = strconv.Atoi(split_str)
		if err != nil {
			return quote("failed"), "", STATUS_FAIL, errors.New("arg: split, should be int")
		}
	}

	meta_info, err := GetDictShardMetaInfos(name, version, depend, shard, split)
	if err != nil {
		return quote("failed"), "", STATUS_FAIL, fmt.Errorf("info error: %v", err)
	}

	b, err := json.Marshal(meta_info)
	if err != nil {
		return quote("failed"), "", STATUS_FAIL, fmt.Errorf("json marshal failed, %v", err)
	}

	return string(b), "", STATUS_OK, nil
}

func GetDictDeployHistory(subpath string, m map[string]string) (string, string, int, error) {
	var deployVerisonInfos []dict.DictVersionInfo
	deployVerisonInfos = Dict.CurrentVersionInfo
	if Dict.WaitVersionInfo.Status > dict.Dict_Status_Deploying {
		deployVerisonInfos = append(deployVerisonInfos, Dict.WaitVersionInfo)
	}

	b, err := json.Marshal(deployVerisonInfos)
	if err != nil {
		return quote("failed"), "", STATUS_FAIL, fmt.Errorf("json marshal failed, %v", err)
	}

	return string(b), "", STATUS_OK, nil
}

func GetDictShardMetaInfos(dictName string, version int, depend int,
	shard int, split int) (dict.DictShardMetaInfo, error) {
	var info dict.DictShardMetaInfo

	for _, v := range Dict.CurrentVersionInfo {
		if v.Version == version && v.Depend == depend {
			if meta, ok := v.MetaInfos[shard]; ok {
				info.Name = dictName;
				info.Version = version
				info.Depend = depend
				info.Shard = shard
				info.Split = split
				info.Meta = meta
				return info, nil
			} else {
				return info, fmt.Errorf("shard not right")
			}
		}
	}
	return info, fmt.Errorf("info not right")
}