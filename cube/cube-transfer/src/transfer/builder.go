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
	"strconv"
	"strings"
	"time"
	"transfer/dict"
)

func BuilderStart(versionInfo dict.DictVersionInfo) error {
	fmt.Printf("[entry]start build\n")
	logex.Noticef("[entry]start build")
	if strings.HasPrefix(Dict.DonefileAddress, dict.FTP_HEADER) || strings.HasPrefix(Dict.DonefileAddress, dict.HTTP_HEADER) {
		if strings.HasPrefix(Dict.DonefileAddress, dict.FTP_HEADER){
			s1 := strings.Replace(Dict.DonefileAddress, dict.FTP_HEADER,"",1)
			index:=strings.Index(s1,"/")
			versionInfo.Input = dict.FTP_HEADER + s1[0 : index] + versionInfo.Input
		}
		if strings.HasPrefix(Dict.DonefileAddress, dict.HTTP_HEADER){
			s1 := strings.Replace(Dict.DonefileAddress, dict.HTTP_HEADER,"",1)
			index:=strings.Index(s1,"/")
			versionInfo.Input = dict.HTTP_HEADER + s1[0 : index] + versionInfo.Input
		}

		localInputPath := Dict.TmpAddress + "/../input/" + Dict.DictName + "_" + strconv.Itoa(versionInfo.Version) + "_" + strconv.Itoa(versionInfo.Depend)
		Wget(versionInfo.Input, localInputPath)
		versionInfo.Input = localInputPath
	}
	Dict.WaitVersionInfo.Output = BuildIndex(versionInfo)
	InitAllInstances()

	return nil
}

func BuildIndex(versionInfo dict.DictVersionInfo) string {
	versionStr := strconv.Itoa(versionInfo.Version)
	dependStr := strconv.Itoa(versionInfo.Depend)

	var params []string
	params = append(params, "-dict_name="+Dict.DictName)
	params = append(params, "-job_mode="+Dict.WaitVersionInfo.Mode)
	curlen := len(Dict.CurrentVersionInfo)
	lastVersion := "0"
	if curlen > 0 && Dict.WaitVersionInfo.Key == Dict.CurrentVersionInfo[curlen-1].Key {
		lastVersion = strconv.Itoa(Dict.CurrentVersionInfo[curlen-1].Version)
	}

	shardNum := strconv.Itoa(Dict.ShardNum)
	params = append(params, "-last_version="+lastVersion)
	params = append(params, "-cur_version="+versionStr)
	params = append(params, "-depend_version="+dependStr)
	params = append(params, "-input_path="+versionInfo.Input)
	params = append(params, "-output_path="+Dict.OutputAddress)
	params = append(params, "-shard_num="+shardNum)
	params = append(params, "-master_address="+TransferAddr+":"+Port)
	params = append(params, "-only_build=false")
	err := ExeCommad(BuildToolLocal, params)
	if err != nil {
		fmt.Printf("build exe: %v\n", err)
		logex.Noticef("[builder] exe cmd err: %v", err)
	}

	outPut := Dict.OutputAddress + "/" + dependStr + "_" + versionStr
	return outPut
}

func InitAllInstances() {
	for i, _ := range Dict.Instances {
		Dict.Instances[i].Status = dict.Instance_Status_Init
		Dict.Instances[i].BuildedTime = int(time.Now().Unix())
		Dict.Instances[i].DownloadStartTime = 0
		Dict.Instances[i].DownloadedTime = 0
		Dict.Instances[i].ReloadStartTime = 0
		Dict.Instances[i].ReloadedTime = 0
		Dict.Instances[i].EnablStartTime = 0
		Dict.Instances[i].EnabledTime = 0
	}
	Dict.DownloadSuccInsts = 0
	Dict.ReloadSuccInsts = 0
	Dict.EnableSuccInsts = 0
}
