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

type DictInfo struct {
	DictName           string            `json:"dict_name"`
	DictMode           string            `json:"dict_mode"`
	ShardNum           int               `json:"shard_num"`
	CopyNum            int               `json:"copy_num"`
	InstancesNum       int               `json:"inst_num"`
	DeployPath         string            `json:"deploy_path"`
	DonefileAddress    string            `json:"donefile_addr"`
	OutputAddress      string            `json:"output_addr"`
	TmpAddress         string            `json:"tmp_addr"`
	DownloadMode       string            `json:"download_mode"`
	WgetPort           string            `json:"wget_port"`
	DownloadSuccInsts  int               `json:"download_inst"`
	ReloadSuccInsts    int               `json:"reload_insts"`
	EnableSuccInsts    int               `json:"enable_insts"`
	Instances          []DictInstance    `json:"instances"`
	WaitVersionInfo    DictVersionInfo   `json:"wait_version_info"`
	CurrentVersionInfo []DictVersionInfo `json:"current_version_info"`
}