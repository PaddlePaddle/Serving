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
	"reflect"
	"strconv"
	"transfer/dict"
)

func initPostHandlers() {
	postHandler = map[string]handlerFunc{
		"/dict/meta_info/register":                     PostDictShardMetaInfoRegister,
	}
}

func PostDictShardMetaInfoRegister(subpath string, m map[string]string) (string, string, int, error) {
	var (
		shardMetaInfo dict.DictShardMetaInfo
		ok            bool
		err           error
		metaInfo      dict.MetaInfo
	)

	shardMetaInfo.Name, ok = m["name"]
	if !ok {
		return quote("failed"), "", STATUS_FAIL, errors.New("need arg: name")
	}

	shardMetaInfo.Version, err = strconv.Atoi(m["version"])
	if err != nil {
		return quote("failed"), "", STATUS_FAIL, errors.New("need arg: version, should be int")
	}

	shardMetaInfo.Depend, err = strconv.Atoi(m["depend"])
	if err != nil {
		return quote("failed"), "", STATUS_FAIL, errors.New("need arg: depend, should be int")
	}

	shardMetaInfo.Shard, err = strconv.Atoi(m["shard"])
	if err != nil {
		return quote("failed"), "", STATUS_FAIL, errors.New("need arg: shard, should be int")
	}
	shardMetaInfo.Split = 1
	split_str, ok := m["split"]
	if ok {
		shardMetaInfo.Split, err = strconv.Atoi(split_str)
		if err != nil {
			return quote("failed"), "", STATUS_FAIL, errors.New("arg: split, should be int")
		}
	}

	// check dict shard_num and status


	if shardMetaInfo.Shard < 0 || shardMetaInfo.Shard >= Dict.ShardNum {
		return quote("failed"), "", STATUS_FAIL, fmt.Errorf("shard value invalid, dict shard is :%v", Dict.ShardNum)
	}

	shardMetaInfo.Meta, ok = m["meta"]
	if !ok {
		return quote("failed"), "", STATUS_FAIL, errors.New("need arg: meta")
	}
	if err = json.Unmarshal([]byte(shardMetaInfo.Meta), &metaInfo); err != nil {
		return quote("failed"), "", STATUS_FAIL, fmt.Errorf("meta string bad formatted: %v", err)
	}
	if reflect.DeepEqual(metaInfo, (dict.MetaInfo{})) {
		return quote("failed"), "", STATUS_FAIL, errors.New("meta string bad formatted")
	}
	if 0 == len(metaInfo.IndexLenList) {
		return quote("failed"), "", STATUS_FAIL, errors.New("meta string bad formatted, index_len_list is null")
	}
	if 0 == len(metaInfo.DataLenList) {
		return quote("failed"), "", STATUS_FAIL, errors.New("meta string bad formatted, data_len_list is null")
	}
	fmt.Printf("update meta shardMetaInfo:%v metaInfo:%v\n", shardMetaInfo,metaInfo)

	if err = UpdateDictShardMetaInfo(shardMetaInfo); err != nil {
		return quote("failed"), "", STATUS_FAIL, fmt.Errorf("update dict_shard_meta_info failed, %v", err)
	}
	fmt.Printf("update meta2\n")

	return quote("ok"), "", STATUS_OK, nil
}

func UpdateDictShardMetaInfo(shardMetaInfo dict.DictShardMetaInfo) error {
	Dict.WaitVersionInfo.MetaInfos[shardMetaInfo.Shard] = shardMetaInfo.Meta
	WriteWaitVersionInfoToFile()
	return nil
}