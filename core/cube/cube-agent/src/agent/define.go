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

package agent

import (
	"errors"
	_ "github.com/Badangel/logex"
	"strings"
	"sync"
)

var (
	Dir           string
	WorkerNum     int
	QueueCapacity int32
	MasterHost    []string
	MasterPort    []string

	TestHostname string
	TestIdc      string
	ShardLock    sync.RWMutex

	CmdWorkPool   *WorkPool
	CmdWorkFilter sync.Map
)

type (
	Status struct {
		Status  string `json:"status"`
		Version string `json:"version"`
	}

	MasterResp struct {
		Success string `json:"success"`
		Message string `json:"message"`
		Data    string `json:"data"`
	}

	ShardInfo struct {
		DictName   string
		ShardSeq   int
		SlotIdList string
		DataDir    string
		Service    string `json:"service,omitempty"`
		Libcube    string `json:"libcube,omitempty"`
	}

	CubeResp struct {
		Status     int    `json:"status"`
		CurVersion string `json:"cur_version"`
		BgVersion  string `json:"bg_version"`
	}
)

var BUILTIN_STATUS = Status{"RUNNING", "3.0.0.1"}

var ShardInfoMap map[string]map[string]*ShardInfo
var disks []string

func GetMaster(master string) (host, port string, err error) {
	if len(ShardInfoMap) < 1 {
		return "", "", errors.New("empty master list.")
	}
	if master == "" {
		return MasterHost[0], MasterPort[0], nil
	}
	if _, ok := ShardInfoMap[master]; ok {
		m := strings.Split(master, ":")
		if len(m) != 2 {
			return MasterHost[0], MasterPort[0], nil
		}
		return m[0], m[1], nil
	} else {
		return MasterHost[0], MasterPort[0], nil
	}
}
