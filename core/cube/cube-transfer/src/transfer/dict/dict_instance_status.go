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

type DictInstance struct {
	DictName          string            `json:"dict_name"`
	Mode              string            `json:"mode"`
	Version           int               `json:"version"`
	Depend            int               `json:"depend"`
	Id                int               `json:"id"`
	Key               int               `json:"key"`
	Shard             int               `json:"shard"`
	Source            string            `json:"source"`
	DeployPath        string            `json:"deploy_path"`
	IP                string            `json:"ip"`
	Port              int               `json:"port"`
	AgentIp           string            `json:"agent_ip"`
	AgentPort         int               `json:"agent_port"`
	Status            InstanceStatus    `json:"status_id"`
	StatusStr         InstanceStatusStr `json:"status"`
	BuildedTime       int               `json:"builded_time"`
	DownloadStartTime int               `json:"download_start_time"`
	DownloadedTime    int               `json:"downloaded_time"`
	ReloadStartTime   int               `json:"reload_start_time"`
	ReloadedTime      int               `json:"reloaded_time"`
	EnablStartTime    int               `json:"enable_start_time"`
	EnabledTime       int               `json:"enabled_time"`
	CreateTime        int               `json:"create_time"`
}

