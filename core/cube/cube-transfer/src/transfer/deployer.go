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
	"time"
	"transfer/dict"
)

func DeployStart(versionInfo dict.DictVersionInfo) error {
	fmt.Printf("[entry]start deploy\n")
	logex.Noticef("[entry]start deploy")
	var err error
	for {
		switch Dict.WaitVersionInfo.Status {
		case dict.Dict_Status_Deploying, dict.Dict_Status_Downloading:
			CmdInstsDownload()
		case dict.Dict_Status_Download_Succ, dict.Dict_Status_Reloading:
			CmdInstsReload()
		case dict.Dict_Status_Reload_Succ, dict.Dict_Status_Enabling:
			CmdInstsEnable()
		default:
			logex.Noticef("dict status %v", Dict.WaitVersionInfo.Status)
		}
		if Dict.WaitVersionInfo.Status == dict.Dict_Status_Finished {
			Dict.WaitVersionInfo.FinishTime = int(time.Now().Unix())
			Dict.CurrentVersionInfo = append(Dict.CurrentVersionInfo, Dict.WaitVersionInfo)
			fmt.Printf("[deploy finish]version:%v\n", Dict.WaitVersionInfo)
			logex.Noticef("[deploy finish]version:%v", Dict.WaitVersionInfo)
			var newVersionInfo dict.DictVersionInfo
			Dict.WaitVersionInfo = newVersionInfo
			WriteCurrentVersionInfoToFile()
			WriteWaitVersionInfoToFile()
			break
		} else {
			time.Sleep(time.Duration(10) * time.Second)
		}
	}
	return err
}

func CmdInstsDownload() {
	for {
		chs := make([]chan error, len(Dict.Instances))
		keyAndRespSlice := make([]dict.CubeAgentResponse, len(Dict.Instances))
		for i, inst := range Dict.Instances {
			if inst.Status != dict.Instance_Status_Download_Succ {
				chs[i] = make(chan error)
				var json_params dict.CubeAgentRequest
				json_params.Command = dict.DOWNLOAD
				json_params.DictName = inst.DictName
				json_params.DeployPath = inst.DeployPath
				json_params.Version = strconv.Itoa(Dict.WaitVersionInfo.Version)
				json_params.Depend = strconv.Itoa(Dict.WaitVersionInfo.Depend)
				json_params.Id = strconv.Itoa(Dict.WaitVersionInfo.Id)
				json_params.Key = strconv.Itoa(Dict.WaitVersionInfo.Key)
				json_params.Mode = Dict.WaitVersionInfo.Mode
				json_params.ShardSeq = inst.Shard
				json_params.Port = strconv.Itoa(inst.Port)
				json_params.Source = dict.GetFileHead(Dict.DownloadMode, TransferAddr, Dict.WgetPort) + Dict.WaitVersionInfo.Output + "/" + json_params.DictName + "_part" + strconv.Itoa(inst.Shard) + ".tar"
				var address = fmt.Sprintf("http://%v:%v/agent/cmd", inst.AgentIp, inst.AgentPort)
				logex.Noticef("[download cmd]%v:%v", address, json_params)
				go nonBlockSendJsonReq("POST2", address, 120, &json_params, &keyAndRespSlice[i], chs[i])
				Dict.Instances[i].DownloadStartTime = int(time.Now().Unix())
				Dict.Instances[i].Mode = Dict.WaitVersionInfo.Mode
			}
		}
		for i, inst := range Dict.Instances {
			if inst.Status != dict.Instance_Status_Download_Succ {
				err := <-chs[i]
				if err != nil || keyAndRespSlice[i].Success != "0" {
					logex.Warningf("cmd cube online downlaod of %v:%v, shard:%v failed", inst.AgentIp, inst.AgentPort, inst.Shard)
					continue
				}
				if inst.Status < dict.Instance_Status_Download_Succ {
					Dict.Instances[i].Status = dict.Instance_Status_Download_Succ
					Dict.Instances[i].DownloadedTime = int(time.Now().Unix())
					Dict.DownloadSuccInsts++
				}
			}
		}
		if Dict.DownloadSuccInsts == Dict.InstancesNum {
			Dict.WaitVersionInfo.Status = dict.Dict_Status_Download_Succ
			fmt.Printf("[all download ok]inst :%v\n", Dict.Instances)
			logex.Noticef("[all download ok]inst :%v", Dict.Instances)
			break
		}
		time.Sleep(5 * time.Second)
	}
}

func CmdInstsReload() {
	for {
		chs := make([]chan error, len(Dict.Instances))
		keyAndRespSlice := make([]dict.CubeAgentResponse, len(Dict.Instances))
		for i, inst := range Dict.Instances {
			if inst.Status != dict.Instance_Status_Reload_Succ {
				chs[i] = make(chan error)
				var json_params dict.CubeAgentRequest
				json_params.Command = dict.RELOAD
				json_params.DictName = inst.DictName
				json_params.DeployPath = inst.DeployPath
				json_params.Version = strconv.Itoa(Dict.WaitVersionInfo.Version)
				json_params.Depend = strconv.Itoa(Dict.WaitVersionInfo.Depend)
				json_params.Id = strconv.Itoa(Dict.WaitVersionInfo.Id)
				json_params.Key = strconv.Itoa(Dict.WaitVersionInfo.Key)
				json_params.Mode = Dict.WaitVersionInfo.Mode
				json_params.ShardSeq = inst.Shard
				json_params.Port = strconv.Itoa(inst.Port)
				json_params.Source = dict.GetFileHead(Dict.DownloadMode, TransferAddr, Dict.WgetPort) + Dict.WaitVersionInfo.Output + "/" + json_params.DictName + "_part" + strconv.Itoa(inst.Shard) + ".tar"

				var address = fmt.Sprintf("http://%v:%v/agent/cmd", inst.AgentIp, inst.AgentPort)
				logex.Noticef("[reload cmd]%v:%v", address, json_params)
				go nonBlockSendJsonReq("POST2", address, 120, &json_params, &keyAndRespSlice[i], chs[i])
				Dict.Instances[i].ReloadStartTime = int(time.Now().Unix())
			}
		}
		for i, inst := range Dict.Instances {
			if inst.Status != dict.Instance_Status_Reload_Succ {
				err := <-chs[i]
				logex.Noticef("[instance resp]reload:%v", Dict.Instances)
				if err != nil || keyAndRespSlice[i].Success != "0" {
					logex.Warningf("cmd cube online reload of %v:%v, shard:%v failed", inst.AgentIp, inst.AgentPort, inst.Shard)
					continue
				}
				if inst.Status < dict.Instance_Status_Reload_Succ {
					Dict.Instances[i].Status = dict.Instance_Status_Reload_Succ
					Dict.Instances[i].ReloadedTime = int(time.Now().Unix())
					Dict.ReloadSuccInsts++
				}
			}
		}
		if Dict.ReloadSuccInsts == Dict.InstancesNum {
			Dict.WaitVersionInfo.Status = dict.Dict_Status_Reload_Succ
			fmt.Printf("[all reload ok]inst:%v\n", Dict.Instances)
			logex.Noticef("[all reload ok]inst :%v", Dict.Instances)
			break
		}
		time.Sleep(5 * time.Second)
	}
}

func CmdInstsEnable() {
	for {
		chs := make([]chan error, len(Dict.Instances))
		keyAndRespSlice := make([]dict.CubeAgentResponse, len(Dict.Instances))
		for i, inst := range Dict.Instances {
			if inst.Status != dict.Instance_Status_Enable_Succ {
				chs[i] = make(chan error)
				var json_params dict.CubeAgentRequest
				json_params.Command = dict.ENABLE
				json_params.DictName = inst.DictName
				json_params.DeployPath = inst.DeployPath
				json_params.Version = strconv.Itoa(Dict.WaitVersionInfo.Version)
				json_params.Depend = strconv.Itoa(Dict.WaitVersionInfo.Depend)
				json_params.Id = strconv.Itoa(Dict.WaitVersionInfo.Id)
				json_params.Key = strconv.Itoa(Dict.WaitVersionInfo.Key)
				json_params.Mode = Dict.WaitVersionInfo.Mode
				json_params.ShardSeq = inst.Shard
				json_params.Port = strconv.Itoa(inst.Port)
				json_params.Source = dict.GetFileHead(Dict.DownloadMode, TransferAddr, Dict.WgetPort) + Dict.WaitVersionInfo.Output + "/" + json_params.DictName + "_part" + strconv.Itoa(inst.Shard) + ".tar"

				var address = fmt.Sprintf("http://%v:%v/agent/cmd", inst.AgentIp, inst.AgentPort)
				logex.Noticef("[enable cmd]%v:%v", address, json_params)
				go nonBlockSendJsonReq("POST2", address, 120, &json_params, &keyAndRespSlice[i], chs[i])
				Dict.Instances[i].EnablStartTime = int(time.Now().Unix())
			}
		}
		for i, inst := range Dict.Instances {
			if inst.Status != dict.Instance_Status_Enable_Succ {
				err := <-chs[i]
				logex.Noticef("[instance resp]enable:%v", Dict.Instances)
				if err != nil || keyAndRespSlice[i].Success != "0" {
					logex.Warningf("cmd cube online enable of %v:%v, shard:%v failed", inst.AgentIp, inst.AgentPort, inst.Shard)
					continue
				}
				if inst.Status < dict.Instance_Status_Enable_Succ {
					Dict.Instances[i].Status = dict.Instance_Status_Enable_Succ
					Dict.Instances[i].EnabledTime = int(time.Now().Unix())
					Dict.EnableSuccInsts++
				}
			}
		}
		if Dict.EnableSuccInsts == Dict.InstancesNum {
			Dict.WaitVersionInfo.Status = dict.Dict_Status_Finished
			fmt.Printf("[all enable ok]inst :%v\n", Dict.Instances)
			logex.Noticef("[all enable ok]inst :%v", Dict.Instances)

			break
		}
		time.Sleep(5 * time.Second)
	}
}
