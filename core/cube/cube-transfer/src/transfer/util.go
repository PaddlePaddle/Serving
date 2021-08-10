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
	"bufio"
	"encoding/json"
	"fmt"
	"github.com/Badangel/logex"
	"io"
	"io/ioutil"
	"os/exec"
)

func WriteWaitVersionInfoToFile(){
	waitFilePath := Dict.TmpAddress + "/" + "WaitVersionInfo.json"
	b, err := json.Marshal(Dict.WaitVersionInfo)
	fmt.Printf("wait version %v\n", Dict.WaitVersionInfo)
	if err != nil {
		logex.Fatalf("WaitVersionInfo format error : %v", err)
	}
	err = ioutil.WriteFile(waitFilePath, b, 0644)
	if err != nil {
		logex.Fatalf("write wait verison info error : %v", err)
		return
	}
	return
}

func WriteCurrentVersionInfoToFile(){
	currentFilePath := Dict.TmpAddress + "/" + "CurrentVersionInfo.json"
	b, err := json.Marshal(Dict.CurrentVersionInfo)
	if err != nil {
		logex.Fatalf(" CurrentVersionInfo format error : %v", err)
	}
	err = ioutil.WriteFile(currentFilePath, b, 0644)
	if err != nil {
		logex.Fatalf("write current verison info error : %v", err)
		return
	}
	return
}

func ExeCommad(files string, params []string) (err error) {
	cmd := exec.Command(files, params...)
	fmt.Println(cmd.Args)
	/*if stdout, err := cmd.StdoutPipe(); err != nil {
		logex.Fatalf("%v", err)
		return
	}*/

	stdout, err := cmd.StdoutPipe()
	defer stdout.Close()

	if err != nil {
		fmt.Println(err)
		return
	}


	if err := cmd.Start(); err != nil {
		logex.Fatalf("%v",err)
	}
	reader := bufio.NewReader(stdout)

	/*buf, err := cmd.Output()
	fmt.Printf("output: %s\n",buf)
	fmt.Printf("err: %v\n",err)*/
	for {
		line, err2 := reader.ReadString('\n')
		if err2 != nil || io.EOF == err2 {
			break
		}
		fmt.Printf("[cmd]>%s",line)
	}

	err = cmd.Wait()
	if nil != err {
		fmt.Println(err)
	}

	return nil
}

func Wget(ftpPath string, downPath string) {
	var params []string
	params = append(params, "--limit-rate=100m")
        params = append(params, "-P")
	params = append(params, downPath)
	params = append(params, "-r")
	params = append(params, "-N")
	params = append(params, "-np")
	params = append(params, "-nd")
	params = append(params, "-R")
	params = append(params, "index.html")
	params = append(params, ftpPath)

	err := ExeCommad("wget", params)
	if err != nil {
		fmt.Printf("wget exe: %v\n", err)
	}
}
