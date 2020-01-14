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

package dict

type CubeAgentRequest struct {
	Command           string `json:"command"`
	DictName          string `json:"dict_name"`
	DeployPath        string `json:"deploy_path"`
	Version           string `json:"version"`
	Depend            string `json:"depend"`
	Id                string `json:"id"`
	Key               string `json:"key"`
	Mode              string `json:"mode"`
	ShardSeq          int    `json:"shard_seq"`
	Source            string `json:"source"`
	Service           string `json:"service,omitempty"`
	SlotIdList        string `json:"slot_id_list,omitempty"`
	ActiveVersionList string `json:"active_version_list,omitempty"`
	Port              string `json:"port,omitempty"`
	VersionSign       string `json:"version_sign,omitempty"`
}


type CubeAgentResponse struct {
    Success string `json:"success"`
    Message string `json:"message"`
    Data    string `json:"data"`
}

