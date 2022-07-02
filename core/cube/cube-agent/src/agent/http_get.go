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
)

func initGetHandlers() {
	getHandler = map[string]handlerFunc{
		"/agent/status": GetStatus,
	}
}

func GetStatus(subpath string, m map[string]string, b []byte) (string, string, error) {
	b, err := json.Marshal(BUILTIN_STATUS)
	if err != nil {
		return quote(""), "", fmt.Errorf("json marshal failed, %v", err)
	}

	return string(b), "", err
}
