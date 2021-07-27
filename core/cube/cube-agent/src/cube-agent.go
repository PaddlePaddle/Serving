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
	"agent"
	"fmt"
	"github.com/Badangel/logex"
	"github.com/docopt/docopt-go"
	"os"
	"path/filepath"
	"runtime"
	"strconv"
)

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())

	agent.Dir, _ = filepath.Abs(filepath.Dir(os.Args[0]))
	usage := fmt.Sprintf(`Usage: ./m_master [options]

Options:
  -n WORKERNUM    set worker num.
  -q QUEUENUM     set queue num.
  -P LISTEN_PORT  agent listen port

Log options:
  -l LOG_LEVEL      set log level, values: 0,1,2,4,8,16. [default: 16]
  --log_dir=DIR     set log output dir. [default: ./log]
  --log_name=NAME   set log name. [default: m_agent]`, agent.Dir)

	opts, err := docopt.Parse(usage, nil, true, "Cube Agent Checker 1.0.0", false)
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

	logex.Notice("--- NEW SESSION -------------------------")
	logex.Notice(">>> log_level:", log_level)

	agent.WorkerNum = 10
	if opts["-n"] != nil {
		n, err := strconv.Atoi(opts["-n"].(string))
		if err == nil {
			agent.WorkerNum = n
		}
	}

	agent.QueueCapacity = 20
	if opts["-q"] != nil {
		q, err := strconv.Atoi(opts["-q"].(string))
		if err == nil {
			agent.QueueCapacity = int32(q)
		}
	}

	agent.CmdWorkPool = agent.NewWorkPool(agent.WorkerNum, agent.QueueCapacity)

	if opts["-P"] == nil {
		logex.Fatalf("ERROR: -P LISTEN PORT must be set!")
		os.Exit(255)
	}

	agentPort := opts["-P"].(string)
	logex.Notice(">>> starting server...")
	addr := ":" + agentPort

	if agent.StartHttp(addr) != nil {
		logex.Noticef("cant start http(addr=%v). quit.", addr)
		os.Exit(0)
	}
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
