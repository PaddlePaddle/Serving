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

import "errors"

var (
	// Dict Mode
	BASE_ONLY = "base_only"
	BASR_DELTA = "base_delta"

	// Deploy Mode
	BASE = "base"
	DELTA = "delta"

	// Succ or Failed Status
	SUCC   = "succ"
	FAILED = "failed"

	//command
	DOWNLOAD = "download"
	RELOAD = "reload"
	ENABLE = "enable"

	FTP_HEADER = "ftp://"
	HTTP_HEADER = "http://"
)

// Dict Status
type DictStatus int

const (
	// Dict Status
	//clear状态编码参考InstanceStatus
	Dict_Status_Clearing      DictStatus = 1
	Dict_Status_Cleared       DictStatus = 2
	Dict_Status_Trigging      DictStatus = 10
	Dict_Status_Building      DictStatus = 20
	Dict_Status_Deploying     DictStatus = 30
	Dict_Status_Downloading   DictStatus = 40
	Dict_Status_Download_Succ DictStatus = 50
	Dict_Status_Reloading     DictStatus = 60
	Dict_Status_Reload_Succ   DictStatus = 70
	Dict_Status_Enabling      DictStatus = 80
	Dict_Status_Finished      DictStatus = 90
	Dict_Status_Restarting    DictStatus = 100
)

func (this DictStatus) String() DictStatusStr {
	switch this {
	case Dict_Status_Trigging:
		return Dict_Status_Trigging_Str
	case Dict_Status_Building:
		return Dict_Status_Building_Str
	case Dict_Status_Deploying:
		return Dict_Status_Deploying_Str
	case Dict_Status_Downloading:
		return Dict_Status_Downloading_Str
	case Dict_Status_Download_Succ:
		return Dict_Status_Download_Succ_Str
	case Dict_Status_Reloading:
		return Dict_Status_Reloading_Str
	case Dict_Status_Reload_Succ:
		return Dict_Status_Reload_Succ_Str
	case Dict_Status_Enabling:
		return Dict_Status_Enabling_Str
	case Dict_Status_Finished:
		return Dict_Status_Finished_Str
	case Dict_Status_Restarting:
		return Dict_Status_Restarting_Str
	case Dict_Status_Clearing:
		return Dict_Status_Clearing_Str
	case Dict_Status_Cleared:
		return Dict_Status_Cleared_Str
	default:
		return ""
	}
}

type DictStatusStr string

const (
	// Dict Status
	Dict_Status_Trigging_Str      DictStatusStr = "Trigging"
	Dict_Status_Building_Str      DictStatusStr = "Building"
	Dict_Status_Deploying_Str     DictStatusStr = "deploying"
	Dict_Status_Downloading_Str   DictStatusStr = "downloading"
	Dict_Status_Download_Succ_Str DictStatusStr = "download_succ"
	Dict_Status_Reloading_Str     DictStatusStr = "reloading"
	Dict_Status_Reload_Succ_Str   DictStatusStr = "reload_succ"
	Dict_Status_Enabling_Str      DictStatusStr = "enabling"
	Dict_Status_Finished_Str      DictStatusStr = "finished"
	Dict_Status_Restarting_Str    DictStatusStr = "restarting"
	Dict_Status_Clearing_Str      DictStatusStr = "clearing"
	Dict_Status_Cleared_Str       DictStatusStr = "cleared"
)

func (this DictStatusStr) Int() (DictStatus, error) {
	switch this {
	case Dict_Status_Trigging_Str:
		return Dict_Status_Trigging, nil
	case Dict_Status_Building_Str:
		return Dict_Status_Building, nil
	case Dict_Status_Deploying_Str:
		return Dict_Status_Deploying, nil
	case Dict_Status_Downloading_Str:
		return Dict_Status_Downloading, nil
	case Dict_Status_Download_Succ_Str:
		return Dict_Status_Download_Succ, nil
	case Dict_Status_Reloading_Str:
		return Dict_Status_Reloading, nil
	case Dict_Status_Reload_Succ_Str:
		return Dict_Status_Reload_Succ, nil
	case Dict_Status_Enabling_Str:
		return Dict_Status_Enabling, nil
	case Dict_Status_Finished_Str:
		return Dict_Status_Finished, nil
	case Dict_Status_Restarting_Str:
		return Dict_Status_Restarting, nil
	case Dict_Status_Clearing_Str:
		return Dict_Status_Clearing, nil
	case Dict_Status_Cleared_Str:
		return Dict_Status_Cleared, nil
	default:
		return 0, errors.New("invalid dict status")
	}
}

// Instance Status:
type InstanceStatus int

const (
	//各种状态都有可能进入clear状态，因此clear相关的状态都小于init状态
	Instance_Status_Clear           InstanceStatus = 1
	Instance_Status_Clearing        InstanceStatus = 2
	Instance_Status_Clear_Failed    InstanceStatus = 3
	Instance_Status_Clear_Succ      InstanceStatus = 4
	Instance_Status_Init            InstanceStatus = 10
	Instance_Status_Downloading     InstanceStatus = 20
	Instance_Status_Download_Failed InstanceStatus = 30
	Instance_Status_Download_Succ   InstanceStatus = 40
	Instance_Status_Reloading       InstanceStatus = 50
	Instance_Status_Reload_Failed   InstanceStatus = 60
	Instance_Status_Reload_Succ     InstanceStatus = 70
	Instance_Status_Enabling        InstanceStatus = 80
	Instance_Status_Enable_Failed   InstanceStatus = 90
	Instance_Status_Enable_Succ     InstanceStatus = 100
	Instance_Status_Poping          InstanceStatus = 110
	Instance_Status_Pop_Failed      InstanceStatus = 120
	Instance_Status_Pop_Succ        InstanceStatus = 130
	Instance_Status_Dead            InstanceStatus = 250
)

func (this InstanceStatus) String() InstanceStatusStr {
	switch this {
	case Instance_Status_Init:
		return Instance_Status_Init_Str
	case Instance_Status_Downloading:
		return Instance_Status_Downloading_Str
	case Instance_Status_Download_Failed:
		return Instance_Status_Download_Failed_Str
	case Instance_Status_Download_Succ:
		return Instance_Status_Download_Succ_Str
	case Instance_Status_Reloading:
		return Instance_Status_Reloading_Str
	case Instance_Status_Reload_Failed:
		return Instance_Status_Reload_Failed_Str
	case Instance_Status_Reload_Succ:
		return Instance_Status_Reload_Succ_Str
	case Instance_Status_Enabling:
		return Instance_Status_Enabling_Str
	case Instance_Status_Enable_Failed:
		return Instance_Status_Enable_Failed_Str
	case Instance_Status_Enable_Succ:
		return Instance_Status_Enable_Succ_Str
	case Instance_Status_Dead:
		return Instance_Status_Dead_Str
	case Instance_Status_Clear:
		return Instance_Status_Clear_Str
	case Instance_Status_Clearing:
		return Instance_Status_Clearing_Str
	case Instance_Status_Clear_Failed:
		return Instance_Status_Clear_Failed_Str
	case Instance_Status_Clear_Succ:
		return Instance_Status_Clear_Succ_Str
	case Instance_Status_Poping:
		return Instance_Status_Poping_Str
	case Instance_Status_Pop_Failed:
		return Instance_Status_Pop_Failed_Str
	case Instance_Status_Pop_Succ:
		return Instance_Status_Pop_Succ_Str
	default:
		return ""
	}
}

type InstanceStatusStr string

const (
	Instance_Status_Init_Str            InstanceStatusStr = "init"
	Instance_Status_Downloading_Str     InstanceStatusStr = "downloading"
	Instance_Status_Download_Failed_Str InstanceStatusStr = "download_failed"
	Instance_Status_Download_Succ_Str   InstanceStatusStr = "download_succ"
	Instance_Status_Reloading_Str       InstanceStatusStr = "reloading"
	Instance_Status_Reload_Failed_Str   InstanceStatusStr = "finish_reload_failed"
	Instance_Status_Reload_Succ_Str     InstanceStatusStr = "finish_reload_succ"
	Instance_Status_Enabling_Str        InstanceStatusStr = "enabling"
	Instance_Status_Enable_Failed_Str   InstanceStatusStr = "enable_failed"
	Instance_Status_Enable_Succ_Str     InstanceStatusStr = "enable_succ"
	Instance_Status_Dead_Str            InstanceStatusStr = "dead"
	Instance_Status_Clear_Str           InstanceStatusStr = "clear"
	Instance_Status_Clearing_Str        InstanceStatusStr = "clearing"
	Instance_Status_Clear_Failed_Str    InstanceStatusStr = "clear_failed"
	Instance_Status_Clear_Succ_Str      InstanceStatusStr = "clear_succ"
	Instance_Status_Poping_Str          InstanceStatusStr = "poping"
	Instance_Status_Pop_Failed_Str      InstanceStatusStr = "pop_failed"
	Instance_Status_Pop_Succ_Str        InstanceStatusStr = "pop_succ"
)

func (this InstanceStatusStr) Int() (InstanceStatus, error) {
	switch this {
	case Instance_Status_Init_Str:
		return Instance_Status_Init, nil
	case Instance_Status_Downloading_Str:
		return Instance_Status_Downloading, nil
	case Instance_Status_Download_Failed_Str:
		return Instance_Status_Download_Failed, nil
	case Instance_Status_Download_Succ_Str:
		return Instance_Status_Download_Succ, nil
	case Instance_Status_Reloading_Str:
		return Instance_Status_Reloading, nil
	case Instance_Status_Reload_Failed_Str:
		return Instance_Status_Reload_Failed, nil
	case Instance_Status_Reload_Succ_Str:
		return Instance_Status_Reload_Succ, nil
	case Instance_Status_Enabling_Str:
		return Instance_Status_Enabling, nil
	case Instance_Status_Enable_Failed_Str:
		return Instance_Status_Enable_Failed, nil
	case Instance_Status_Enable_Succ_Str:
		return Instance_Status_Enable_Succ, nil
	case Instance_Status_Dead_Str:
		return Instance_Status_Dead, nil
	case Instance_Status_Clear_Str:
		return Instance_Status_Clear, nil
	case Instance_Status_Clearing_Str:
		return Instance_Status_Clearing, nil
	case Instance_Status_Clear_Failed_Str:
		return Instance_Status_Clear_Failed, nil
	case Instance_Status_Clear_Succ_Str:
		return Instance_Status_Clear_Succ, nil
	case Instance_Status_Poping_Str:
		return Instance_Status_Poping, nil
	case Instance_Status_Pop_Failed_Str:
		return Instance_Status_Pop_Failed, nil
	case Instance_Status_Pop_Succ_Str:
		return Instance_Status_Pop_Succ, nil
	default:
		return 0, errors.New("invalid instance status")
	}
}