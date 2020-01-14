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
	"fmt"
	"github.com/Badangel/logex"
	"io/ioutil"
	"strconv"
	"strings"
	"time"
	"transfer/dict"
)

func TriggerStart(addr string) (version dict.DictVersionInfo, err error) {
	return GetDoneFileInfo(addr)
}

func GetDoneFileInfo(addr string) (version dict.DictVersionInfo, err error) {
	fmt.Printf("[entry]start trigger\n")
	logex.Noticef("[entry]start trigger")
	//if donefile is in ftp, first download to local
	if strings.HasPrefix(addr, dict.FTP_HEADER) || strings.HasPrefix(addr, dict.HTTP_HEADER) {
		donefileAddr := Dict.TmpAddress + "/../donefile"
		Wget(addr, donefileAddr)
		addr = donefileAddr
	}

	baseDonefile := addr + "/base.txt"
	fmt.Printf("[trigrer]donefile path:%v \n", baseDonefile)
	logex.Noticef("[trigrer]base donefile path:%v", baseDonefile)
	contents, err := ioutil.ReadFile(baseDonefile)
	VersionLen := len(Dict.CurrentVersionInfo)
	version.DictName = Dict.DictName
	if err != nil {
		fmt.Printf("[trigrer]read files err:%v \n", err)
		logex.Fatalf("[trigrer]read files err:%v ", err)
		return
	} else {
		contentss := string(contents)
		lines := strings.Split(contentss, "\n")
		index := len(lines) - 1
		//one line length smaller than 3 maybe blank or return
		for len(lines[index]) < 3 {
			index--
		}
		if index < 0 {
			logex.Noticef("[trigrer]get base donfile info error")
			err = fmt.Errorf("[trigrer]get base donfile info error")
			return
		}
		var donefileInfo dict.DonefileInfo
		fmt.Printf("line %v: %v\n", index, lines[index])
		if err = json.Unmarshal([]byte(lines[index]), &donefileInfo); err != nil {
			return
		}
		logex.Noticef("[trigrer]donfile info:%v", donefileInfo)
		newId, _ := strconv.Atoi(donefileInfo.Id)
		if VersionLen == 0 || newId > Dict.CurrentVersionInfo[VersionLen-1].Key {
			version.Id = newId
			version.Key, _ = strconv.Atoi(donefileInfo.Key)
			version.Input = donefileInfo.Input
			deployVersion := int(time.Now().Unix())
			version.CreateTime = deployVersion
			version.Version = deployVersion
			version.Depend = deployVersion
			version.Mode = dict.BASE
			return
		}
	}
	if Dict.DictMode == dict.BASR_DELTA && VersionLen > 0 {
		patchDonefile := addr + "/patch.txt"
		fmt.Printf("[trigrer]patchDonefile path:%v \n", patchDonefile)
		logex.Noticef("[trigrer]patch donefile path:%v", patchDonefile)
		contents, err = ioutil.ReadFile(patchDonefile)
		if err != nil {
			fmt.Printf("read files err:%v \n", err)
			return
		} else {
			contentss := string(contents)
			lines := strings.Split(contentss, "\n")

			for index := 0; index < len(lines)-1; index++ {
				if len(lines[index]) < 3 {
					logex.Noticef("[trigrer]get patch donfile info error")
					err = fmt.Errorf("[trigrer]get patch donfile info error")
					return
				}
				var donefileInfo dict.DonefileInfo
				if err = json.Unmarshal([]byte(lines[index]), &donefileInfo); err != nil {
					return
				}
				logex.Noticef("[trigrer]donfile info:%v", donefileInfo)
				newId, _ := strconv.Atoi(donefileInfo.Id)
				newKey, _ := strconv.Atoi(donefileInfo.Key)
				if newId > Dict.CurrentVersionInfo[VersionLen-1].Id && newKey == Dict.CurrentVersionInfo[VersionLen-1].Key {
					version.Id = newId
					version.Key, _ = strconv.Atoi(donefileInfo.Key)
					version.Input = donefileInfo.Input
					deployVersion := int(time.Now().Unix())
					version.CreateTime = deployVersion
					version.Version = deployVersion
					version.Depend = Dict.CurrentVersionInfo[VersionLen-1].Depend
					version.Mode = dict.DELTA
					return
				}
			}
		}
	}
	return
}
