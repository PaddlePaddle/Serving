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
	"fmt"
	"github.com/Badangel/logex"
	"transfer/dict"
)

func Start() {

	BackupTransfer()
}

func BackupTransfer() {
	//trigger
	version, err := TriggerStart(Dict.DonefileAddress)
	if err != nil {
		logex.Fatalf("[trigger err]trigger err:%v ", err)
		fmt.Printf("[error]trigger err:%v \n", err)
		fmt.Print("transfer over!")
        	logex.Noticef("[transfer]status machine exit!")
                return
	}
	logex.Noticef("[trigger] get version:%v \n", version)
        Dict.WaitVersionInfo = version
	logex.Noticef("[trigger finish] WaitVersionInfo version:%v \n", Dict.WaitVersionInfo)
	WriteWaitVersionInfoToFile()
        
	//builder
	Dict.WaitVersionInfo.Status = dict.Dict_Status_Building
	Dict.WaitVersionInfo.MetaInfos = make(map[int]string)
	WriteWaitVersionInfoToFile()
	if err = BuilderStart(Dict.WaitVersionInfo); err != nil {
		logex.Fatalf("builder err:%v \n", err)
	}

	if Dict.WaitVersionInfo.Mode == dict.BASE {
		var newCurrentVersion []dict.DictVersionInfo
		Dict.CurrentVersionInfo = newCurrentVersion
		WriteCurrentVersionInfoToFile()
	}
        if Dict.WaitVersionInfo.Mode == dict.DELTA {
                var newCurrentVersion []dict.DictVersionInfo
                Dict.CurrentVersionInfo = newCurrentVersion
                WriteCurrentVersionInfoToFile()
        }
	logex.Noticef("[builder finish] WaitVersionInfo version:%v \n", Dict.WaitVersionInfo)

	//deployer
	Dict.WaitVersionInfo.Status = dict.Dict_Status_Deploying
	WriteWaitVersionInfoToFile()
	if err = DeployStart(Dict.WaitVersionInfo); err != nil {
		logex.Fatalf("deploy err:%v \n", err)
	}
        logex.Noticef("[deploy finish]current version: %v\n",Dict.CurrentVersionInfo)
	fmt.Print("transfer over!")
	logex.Noticef("[transfer]status machine exit!")
}
