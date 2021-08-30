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

package main

import (
	"fmt"
	"github.com/Badangel/logex"
	"github.com/docopt/docopt-go"
	"os"
	"path/filepath"
	"runtime"
	"strconv"
	"strings"
	"time"
	"transfer"
	"transfer/dict"
)

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())
	transfer.Dir, _ = filepath.Abs(filepath.Dir(os.Args[0]))

	usage := fmt.Sprintf(`Usage: ./m_master [options]

Options:
  -p PORT         set listen port. [default: 8099]
  --config=conf/transfer.conf       set conf file. [defalut: ./conf/transfer.conf]

Log options:
  -l LOG_LEVEL      set log level, values: 0,1,2,4,8,16. [default: 4]
  --log_dir=DIR     set log output dir. [default: ./log]
  --log_name=NAME   set log name. [default: transfer]`,
		transfer.Dir)
	opts, err := docopt.Parse(usage, nil, true, "Cube Transfer", false)
	if err != nil {
		fmt.Println("ERROR:", err)
		os.Exit(1)
	}
	log_level, _ := strconv.Atoi(opts["-l"].(string))
	log_name := opts["--log_name"].(string)
	log_dir := opts["--log_dir"].(string)
	logex.SetLevel(getLogLevel(log_level))
	if err := logex.SetUpFileLogger(log_dir, log_name, nil); err != nil {
		fmt.Println("ERROR:", err)
	}

	fmt.Printf("%v: Print stdout1 here...\n", time.Now())
	os.Stderr.WriteString(fmt.Sprintf("%v: Print stderr here...\n", time.Now()))

	logex.Notice("--- NEW SESSION -------------------------")
	logex.Notice(">>> log_level:", log_level)

	// settings:
	if opts["-p"] == nil {
		logex.Fatal("ERROR: -p PORT must be set!")
		fmt.Fprintln(os.Stderr, "ERROR: -p PORT must be set!")
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(1)
	}
	transfer.Port = opts["-p"].(string)
	logex.Notice(">>> port:", transfer.Port)

	if opts["--config"] == nil {
		logex.Fatal("ERROR: --config config_file must be set!")
		fmt.Fprintln(os.Stderr, "ERROR: --config config_file must be set!")
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(1)
	}
	//read conf
	var configMgr transfer.ConfigManager
	configMgr.Init(opts["--config"].(string))
	transfer.Dict.DictName = configMgr.Read("default", "dict_name")
	if transfer.Dict.DictName == "" {
		logex.Fatal("ERROR: nead [default] DictName in config_file!")
		fmt.Fprintln(os.Stderr, "ERROR: nead [default] DictName in config_file!")
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(1)
	}
	logex.Notice(">>> DictName:", transfer.Dict.DictName)

	transfer.Dict.DictMode = configMgr.Read("default", "mode")
	if transfer.Dict.DictMode == "" {
		logex.Fatal("ERROR: nead [default] DictMode in config_file!")
		fmt.Fprintln(os.Stderr, "ERROR: nead [default] DictMode in config_file!")
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(1)
	}
	logex.Notice(">>> Mode:", transfer.Dict.DictMode)

	transfer.Dict.DownloadMode = configMgr.Read("default", "download_mode")
	if transfer.Dict.DownloadMode != "http" && transfer.Dict.DownloadMode != "ftp" {
		logex.Fatal("ERROR: nead [default] download_mode in config_file! only support ftp or http")
		fmt.Fprintln(os.Stderr, "ERROR: nead [default] download_mode in config_file! only support ftp or http")
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(1)
	}
	logex.Notice(">>> DownloadMode:", transfer.Dict.DownloadMode)

	transfer.Dict.WgetPort = configMgr.Read("default", "wget_port")
	if transfer.Dict.WgetPort == "" {
		logex.Fatal("ERROR: nead [default] wget_port in config_file!")
		fmt.Fprintln(os.Stderr, "ERROR: nead [default] wget_port in config_file!")
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(1)
	}
	var wget_port int
	wget_port, err = strconv.Atoi(transfer.Dict.WgetPort)
	if err != nil {
		logex.Fatal("wget_port form is not right need int")
		os.Exit(1)
	}
	logex.Notice(">>> WgetPort:", wget_port)

	transfer.BuildToolLocal = configMgr.Read("default", "buildtool_local")
	if transfer.BuildToolLocal == "" {
		logex.Fatal("ERROR: nead [default] BuildToolLocal in config_file!")
		fmt.Fprintln(os.Stderr, "ERROR: nead [default] BuildToolLocal in config_file!")
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(1)
	}
	logex.Notice(">>> BuildToolLocal:", transfer.BuildToolLocal)

	transfer.Dict.DonefileAddress = configMgr.Read("default", "donefile_address")
	if transfer.Dict.DonefileAddress == "" {
		logex.Fatal("ERROR: nead [default] DonefileAddress in config_file!")
		fmt.Fprintln(os.Stderr, "ERROR: nead [default] DonefileAddress in config_file!")
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(1)
	}
	logex.Notice(">>> DonefileAddress:", transfer.Dict.DonefileAddress)

	transfer.Dict.OutputAddress = configMgr.Read("default", "output_address")
	if transfer.Dict.OutputAddress == "" {
		logex.Fatal("ERROR: nead [default] OutputAddress in config_file!")
		fmt.Fprintln(os.Stderr, "ERROR: nead [default] OutputAddress in config_file!")
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(1)
	}
	logex.Notice(">>> OutputAddress:", transfer.Dict.OutputAddress)

	transfer.Dict.TmpAddress = configMgr.Read("default", "tmp_address")
	if transfer.Dict.TmpAddress == "" {
		logex.Fatal("ERROR: nead [default] TmpAddress in config_file!")
		fmt.Fprintln(os.Stderr, "ERROR: nead [default] TmpAddress in config_file!")
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(1)
	}
	logex.Notice(">>> TmpAddress:", transfer.Dict.TmpAddress)

	ShardNumStr := configMgr.Read("default", "shard_num")
	if ShardNumStr == "" {
		logex.Fatal("ERROR: nead [default] ShardNum in config_file!")
		fmt.Fprintln(os.Stderr, "ERROR: nead [default] ShardNum in config_file!")
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(1)
	}
	transfer.Dict.ShardNum, err = strconv.Atoi(ShardNumStr)
	if err != nil {
		logex.Fatal("ShardNum form is not right")
		os.Exit(1)
	}
	logex.Notice(">>> ShardNum:", transfer.Dict.ShardNum)

	CopyNumStr := configMgr.Read("default", "copy_num")
	if CopyNumStr == "" {
		logex.Fatal("ERROR: nead [default] CopyNum in config_file!")
		fmt.Fprintln(os.Stderr, "ERROR: nead [default] CopyNum in config_file!")
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(1)
	}
	transfer.Dict.CopyNum, err = strconv.Atoi(CopyNumStr)
	if err != nil {
		logex.Fatal("ShardNum form is not right")
		os.Exit(1)
	}
	logex.Notice(">>> CopyNum:", transfer.Dict.CopyNum)

	transfer.Dict.InstancesNum = transfer.Dict.ShardNum * transfer.Dict.CopyNum

	transfer.Dict.DeployPath = configMgr.Read("default", "deploy_path")
	if transfer.Dict.DeployPath == "" {
		logex.Fatal("ERROR: nead [default] DeployPath in config_file!")
		fmt.Fprintln(os.Stderr, "ERROR: nead [default] DeployPath in config_file!")
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(1)
	}
	logex.Notice(">>> DeployPath:", transfer.Dict.DeployPath)

	transfer.TransferAddr = configMgr.Read("default", "transfer_address")
	if transfer.TransferAddr == "" {
		logex.Fatal("ERROR: nead [default] TransferAddr in config_file!")
		fmt.Fprintln(os.Stderr, "ERROR: nead [default] TransferAddr in config_file!")
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(1)
	}
	logex.Notice(">>> TransferAddr:", transfer.TransferAddr)

	for i := 0; i < transfer.Dict.ShardNum; i++ {
		for j := 0; j < transfer.Dict.CopyNum; j++ {
			var instance dict.DictInstance
			agentName := fmt.Sprintf("agent%d_%d", i, j)
			agentInfo := configMgr.Read("cube_agent", agentName)
			agentInfoSlice := strings.Split(agentInfo, ":")
			if len(agentInfoSlice) != 2 {
				logex.Fatal("agent conf format not right! sample: ip:port")
				os.Exit(1)
			}
			cubeName := fmt.Sprintf("cube%d_%d", i, j)
			cubeInfo := configMgr.Read("cube_agent", cubeName)
			cubeInfoSlice := strings.Split(cubeInfo, ":")
			if len(cubeInfoSlice) != 3 {
				logex.Fatal("cube conf format not right! sample: ip:port:deploy_path")
				os.Exit(1)
			}
			instance.DictName = transfer.Dict.DictName
			instance.AgentIp = agentInfoSlice[0]
			instance.AgentPort, _ = strconv.Atoi(agentInfoSlice[1])
			instance.IP = cubeInfoSlice[0]
			instance.Port, _ = strconv.Atoi(cubeInfoSlice[1])
			instance.DictName = transfer.Dict.DictName
			instance.CreateTime = int(time.Now().Unix())
			instance.Shard = i
			instance.DeployPath = cubeInfoSlice[2]
			transfer.Dict.Instances = append(transfer.Dict.Instances, instance)
		}
	}
	logex.Noticef(">>> instance: %v", transfer.Dict.Instances)

	transfer.Start()
	fmt.Print("m-cube-transfer over!")

}

func getLogLevel(log_level int) logex.Level {
	switch log_level {
	case 16:
		return logex.DEBUG
	case 8:
		return logex.TRACE
	case 4:
		return logex.NOTICE
	case 2:
		return logex.WARNING
	case 1:
		return logex.FATAL
	case 0:
		return logex.NONE
	}
	return logex.DEBUG
}
