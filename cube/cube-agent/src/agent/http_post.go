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
	"encoding/json"
	"fmt"
	"github.com/Badangel/logex"
)

func initPostHandlers() {
	postHandler = map[string]handlerFunc{
		"/agent/cmd": PostCmd,
	}
}

func PostCmd(subpath string, m map[string]string, b []byte) (string, string, error) {
	var work Work
	err := json.Unmarshal(b, &work)
	if err != nil {
		logex.Warningf("Unmarshal from %s error (+%v)", string(b), err)
		return quote(""), "", fmt.Errorf("Work json unmarshal work failed, %v", err)
	}

	if _, ok := CmdWorkFilter.Load(work.Token()); ok {
		logex.Warningf("Another work with same token is doing. Token(%s)", work.Token())
		return quote(""), "", fmt.Errorf("Another work with same key is doing.", err)
	}

	CmdWorkFilter.Store(work.Token(), true)
	err = work.DoWork()
	CmdWorkFilter.Delete(work.Token())
	if err != nil {
		return quote(""), "", fmt.Errorf("Do work failed.", err)
	}

	return quote(""), "", err
}
