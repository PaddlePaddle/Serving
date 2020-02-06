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
	"crypto/md5"
	"encoding/json"
	"errors"
	"fmt"
	"github.com/Badangel/logex"
	"github.com/Badangel/pipeline"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"time"
)

const (
	COMMAND_DOWNLOAD = "download"
	COMMAND_RELOAD   = "reload"
	COMMAND_SWITCH   = "enable"
	COMMAND_ROLLBACK = "rollback"
	COMMAND_CHECK    = "check"
	COMMAND_CLEAR    = "clear"
	COMMAND_POP      = "pop"

	RETRY_TIMES                   = 3
	REQUEST_MASTER_TIMEOUT_SECOND = 60
	MAX_DOWN_CO                   = 7

	RELOAD_RETRY_TIMES           = 3
	RELOAD_RETRY_INTERVAL_SECOND = 10

	DOWNLOAD_DONE_MARK_FILE = ".download_done"
)

type VersionInfo struct {
	Version string
	Depend  string
	Source  string
}

type Work struct {
	DictName          string        `json:"dict_name"`
	ShardSeq          int           `json:"shard_seq"`
	DeployPath        string        `json:"deploy_path"`
	Command           string        `json:"command"`
	Version           string        `json:"version"`
	Depend            string        `json:"depend"`
	Source            string        `json:"source"`
	Mode              string        `json:"mode"`
	DictMode          string        `json:"dict_mode"`
	Port              string        `json:"port"`
	bRollback         bool          `json:"b_rollback"`
	RollbackInfo      []VersionInfo `json:"rollback_info"`
	Status            string        `json:"status"`
	FinishStatus      string        `json:"finish_status"`
	Service           string        `json:"service,omitempty"`
	VersionSign       string        `json:"version_sign,omitempty"`
	MasterAddress     string        `json:"master_address,omitempty"`
	ActiveVersionList string        `json:"active_version_list,omitempty"`
}

func (work *Work) Token() string {
	return work.DictName + strconv.Itoa(work.ShardSeq) + work.Service
}

func (work *Work) Valid() bool {
	if work.Command == "" ||
		work.Version == "" ||
		work.Depend == "" {
		return false
	}
	return true
}

func (work *Work) DoWork() error {
	var err error
	if !work.Valid() {
		err = errors.New("Work is invalid")
		return err
	}
	switch work.Command {
	case COMMAND_DOWNLOAD:
		err = work.Download()
	case COMMAND_RELOAD:
		err = work.Reload()
	case COMMAND_SWITCH:
		err = work.Enable()
	case COMMAND_CHECK:
		err = work.Check()
	case COMMAND_CLEAR:
		err = work.Clear()
	case COMMAND_POP:
		err = work.Pop()
	default:
		logex.Debugf("Invalid command %s received", work.Command)
		err = errors.New("Invalid command.")
	}
	return err
}

func GetDownloadDirs(dictName, service, version, depend, deployPath string, shardSeq,
	split int) ([]string, error) {
	dirs := make([]string, 0, split)
	if deployPath == "" {
		return dirs, errors.New("Invalid Deploy path")
	}
	parentDir := getParentDir(version, depend)
	if split < 2 {
		disk := path.Join(deployPath, "cube_data")
		if service == "" {
			dirs = append(dirs, path.Join(disk, strconv.Itoa(shardSeq), parentDir))
		} else {
			dirs = append(dirs, path.Join(disk, strconv.Itoa(shardSeq), parentDir+"-"+dictName))
		}
	} else {
		for i := 0; i < split; i++ {
			disk := path.Join(deployPath, "cube_data")
			if service == "" {
				dirs = append(dirs, path.Join(disk, strconv.Itoa(shardSeq), strconv.Itoa(i), parentDir))
			} else {
				dirs = append(dirs, path.Join(disk, strconv.Itoa(shardSeq),
					parentDir+"-"+dictName))
			}
		}
	}

	return dirs, nil
}

func GetDataLinkDirs(dictName, service, version, depend, deployPath string, shardSeq,
	split int) []string {
	dirs := make([]string, 0, split)
	parentDir := getParentDir(version, depend)
	if split < 2 {
		disk := path.Join(deployPath, "data")
		if service == "" {
			dirs = append(dirs, path.Join(disk, parentDir))
		}
	} else {
		for i := 0; i < split; i++ {
			disk := path.Join(deployPath, "data")
			if service == "" {
				dirs = append(dirs, path.Join(disk, strconv.Itoa(i), parentDir))
			}
		}
	}
	return dirs
}

func (work *Work) Download() (err error) {
	err = DoDownload(work.DictName, work.Service, work.Version, work.Depend, work.Mode, work.Source,
		work.DeployPath, work.ShardSeq)

	if err != nil {
		logex.Warningf("download error, failed to download %s, dir is %s, error is (+%v)", work.Source, work.DeployPath, err)
		return
	}

	if work.Service == "" {
		err = UnTar(work.DictName, work.Service, work.Version, work.Depend, work.Source,
			work.DeployPath, work.ShardSeq)

		if err == nil {
			dataPath := path.Join(work.DeployPath, "data")

			// remove all old links
			if work.Mode == "base" || len(work.RollbackInfo) != 0 {
				cmd := fmt.Sprintf("ls -l %s | grep -E 'data.|index.' | awk '{print $9}'", dataPath)
				stdout, _, err := RetryCmd(cmd, RETRY_TIMES)
				if err == nil && stdout != "" {
					fileNameLi := strings.Split(strings.TrimSpace(stdout), "\n")
					for _, file := range fileNameLi {
						err = os.Remove(path.Join(dataPath, file))
						logex.Debugf("os.Remove(%s) error (%+v) ", path.Join(dataPath, file), err)
					}
				}
			}

			// create symbolic link to the version rollbacked
			err = CreateSymlink(work.DictName, work.Service, work.Version, work.Depend, dataPath,
				work.DeployPath, work.ShardSeq, len(strings.Split(work.Source, ";")))
		} else {
			logex.Warningf("download error, failed to untar for %s, dir is %s, error is (+%v)", work.Source, work.DeployPath, err)
		}
	}

	if err == nil {
		// clear history data
		work.clearData()
		work.clearLink()
	} else {
		logex.Warningf("create symlink failed, error is (+%v)", err)
	}

	return
}

func (work *Work) clearData() (err error) {
	split := len(strings.Split(work.Source, ";"))
	downloadDirs, err := GetDownloadDirs(work.DictName, work.Service, work.Version, work.Depend,
		work.DeployPath, work.ShardSeq, split)
	if err != nil {
		logex.Warningf("clearData failed, error is (+%v)", err)
		return
	}
	for _, downloadDir := range downloadDirs {
		parentDir, _ := filepath.Split(downloadDir)

		cmd := fmt.Sprintf("ls -l %s | grep -v %s | awk '{print $9}'", parentDir, work.Depend)

		stdout, _, err := RetryCmd(cmd, RETRY_TIMES)
		if err != nil || stdout == "" || work.Service != "" {
			cmd = fmt.Sprintf("find %s -type d -ctime +1 -print | xargs -i rm -rf {}", parentDir)
			_, _, err = RetryCmd(cmd, RETRY_TIMES)
		} else {
			rmList := strings.Split(strings.TrimSpace(stdout), "\n")
			for i := 0; i < len(rmList); i++ {
				if rmList[i] == "" {
					continue
				}
				cmd = fmt.Sprintf("rm -rf %s/%s*", parentDir, rmList[i])
				_, _, err = RetryCmd(cmd, RETRY_TIMES)
			}
		}
	}

	return
}

func (work *Work) clearPatchData() (err error) {
	if work.Service != "" {
		return
	}
	split := len(strings.Split(work.Source, ";"))
	downloadDirs, err := GetDownloadDirs(work.DictName, work.Service, work.Version, work.Depend,
		work.DeployPath, work.ShardSeq, split)
	if err != nil {
		logex.Warningf("clearPatchData failed, error is (+%v)", err)
		return
	}
	for _, downloadDir := range downloadDirs {
		parentDir, _ := filepath.Split(downloadDir)
		cmd := fmt.Sprintf("ls -l %s | grep %s_ | awk '{print $9}'", parentDir, work.Depend)
		stdout, _, err := RetryCmd(cmd, RETRY_TIMES)
		if err == nil && stdout != "" {
			rmList := strings.Split(strings.TrimSpace(stdout), "\n")
			for i := 0; i < len(rmList); i++ {
				if rmList[i] == "" {
					continue
				}
				cmd = fmt.Sprintf("rm -rf %s/%s*", parentDir, rmList[i])
				_, _, err = RetryCmd(cmd, RETRY_TIMES)
			}
		}
	}

	return
}

func (work *Work) clearLink() (err error) {
	if work.Service != "" {
		return
	}
	split := len(strings.Split(work.Source, ";"))
	dataLinkDirs := GetDataLinkDirs(work.DictName, work.Service, work.Version, work.Depend,
		work.DeployPath, work.ShardSeq, split)
	for _, linkDir := range dataLinkDirs {
		parentDir, _ := filepath.Split(linkDir)
		cmd := fmt.Sprintf("ls -l %s | grep -v %s | awk '{print $9}'", parentDir, work.Depend)

		stdout, _, err := RetryCmd(cmd, RETRY_TIMES)
		if err != nil || stdout == "" {
			cmd = fmt.Sprintf("find %s -type d -ctime +1 -print | xargs -i rm -rf {}", parentDir)
			_, _, err = RetryCmd(cmd, RETRY_TIMES)
		} else {
			rmList := strings.Split(strings.TrimSpace(stdout), "\n")
			for i := 0; i < len(rmList); i++ {
				if rmList[i] == "" {
					continue
				}
				cmd = fmt.Sprintf("rm -rf %s/%s*", parentDir, rmList[i])
				_, _, err = RetryCmd(cmd, RETRY_TIMES)
			}
		}
	}

	return
}

func (work *Work) clearPatchLink() (err error) {
	if work.Service != "" {
		return
	}
	split := len(strings.Split(work.Source, ";"))
	dataLinkDirs := GetDataLinkDirs(work.DictName, work.Service, work.Version, work.Depend,
		work.DeployPath, work.ShardSeq, split)
	for _, linkDir := range dataLinkDirs {
		parentDir, _ := filepath.Split(linkDir)
		cmd := fmt.Sprintf("ls -l %s | grep %s_ | awk '{print $9}'", parentDir, work.Depend)

		stdout, _, err := RetryCmd(cmd, RETRY_TIMES)
		if err == nil && stdout != "" {
			rmList := strings.Split(strings.TrimSpace(stdout), "\n")
			for i := 0; i < len(rmList); i++ {
				if rmList[i] == "" {
					continue
				}
				cmd = fmt.Sprintf("rm -rf %s/%s*", parentDir, rmList[i])
				_, _, err = RetryCmd(cmd, RETRY_TIMES)
			}
		}
	}

	return
}

func UnTar(dictName, service, version, depend, source, deployPath string, shardSeq int) (err error) {
	sources := strings.Split(source, ";")
	downloadDirs, err := GetDownloadDirs(dictName, service, version, depend, deployPath, shardSeq,
		len(sources))
	if err != nil {
		logex.Warningf("UnTar failed, error is (+%v)", err)
		return
	}
	for i := 0; i < len(sources); i++ {
		fileName := GetFileName(sources[i])
		untarCmd := fmt.Sprintf("tar xvf %s -C %s", path.Join(downloadDirs[i], fileName), downloadDirs[i])
		_, _, err = RetryCmd(untarCmd, RETRY_TIMES)
	}

	return
}

func CreateSymlink(dictName, service, version, depend, dataPath, deployPath string, shardSeq,
	split int) (err error) {
	downloadDirs, err := GetDownloadDirs(dictName, service, version, depend, deployPath, shardSeq, split)
	if err != nil {
		logex.Warningf("CreateSymlink failed, error is (+%v)", err)
	}
	for i, downloadDir := range downloadDirs {
		cmd := fmt.Sprintf("ls -l %s | grep -E 'data.|index.' | awk '{print $NF}'", downloadDir)
		stdout, _, err := RetryCmd(cmd, RETRY_TIMES)

		if err == nil && stdout != "" {
			fileNameLi := strings.Split(strings.TrimSpace(stdout), "\n")
			versionDir := getParentDir(version, depend)
			versionFile := path.Join(dataPath, "VERSION")
			dataSubPath := ""
			if split > 1 {
				dataSubPath = path.Join(dataPath, strconv.Itoa(i), versionDir)
			} else {
				dataSubPath = path.Join(dataPath, versionDir)
			}
			if err = os.MkdirAll(dataSubPath, 0755); err != nil {
				// return err
				logex.Warningf("os.Mkdir %s failed, err:[%v]", dataSubPath, err)
			}
			if dataSubPath != "" {
				cmd = fmt.Sprintf("find %s/.. -type d -ctime +5 -print | xargs -i rm -rf {}", dataSubPath)
				_, _, err = RetryCmd(cmd, RETRY_TIMES)
			}
			for _, file := range fileNameLi {
				dataLink := ""
				tempDataPath := ""
				if split > 1 {
					dataLink = path.Join(dataPath, strconv.Itoa(i), file)
					tempDataPath = path.Join(dataPath, strconv.Itoa(i))
				} else {
					dataLink = path.Join(dataPath, file)
					tempDataPath = dataPath
				}
				cmd = fmt.Sprintf("rm -rf %s", dataLink)
				_, stderr, _ := RetryCmd(cmd, RETRY_TIMES)
				logex.Noticef("rm -rf %s, err:[%s]", dataLink, stderr)

				// create new symlink
				err = os.Symlink(path.Join(downloadDir, file), dataLink)
				logex.Noticef("os.Symlink %s %s return (%+v)", path.Join(downloadDir, file), dataLink, err)
				fmt.Println("os.Symlink: ", path.Join(downloadDir, file), dataLink, err)
				cmd = fmt.Sprintf("cp -d %s/index.* %s/", tempDataPath, dataSubPath)
				_, stderr, _ = RetryCmd(cmd, RETRY_TIMES)
				logex.Noticef("cp -d index Symlink to version dir %s, err:[%s]", dataSubPath, stderr)
				cmd = fmt.Sprintf("cp -d %s/data.* %s/", tempDataPath, dataSubPath)
				_, stderr, _ = RetryCmd(cmd, RETRY_TIMES)
				logex.Noticef("cp -d data Symlink to version dir %s, err:[%s]", dataSubPath, stderr)
			}
			cmd = fmt.Sprintf("echo %s > %s", versionDir, versionFile)
			if _, _, err = RetryCmd(cmd, RETRY_TIMES); err != nil {
				return err
			}
		}
	}

	return
}

func (work *Work) CheckToReload() bool {
	statusCmd := fmt.Sprintf("curl -s -d '{\"cmd\":\"status\"}' http://127.0.0.1:%s/ControlService/cmd", work.Port)
	stdout, _, _ := RetryCmd(statusCmd, RETRY_TIMES)
	var resp CubeResp
	json.Unmarshal([]byte(stdout), &resp)
	version := getParentDir(work.Version, work.Depend)

	if resp.CurVersion == "" && resp.BgVersion == "" {
		logex.Noticef("cube version empty")
		return true
	}
	if resp.CurVersion == version || resp.BgVersion == version {
		logex.Noticef("cube version has matched. version: %s", version)
		return false
	}
	return true
}

func (work *Work) Reload() (err error) {
	if work.Port == "" {
		err = errors.New("Reload with invalid port.")
		return
	}
	if !work.CheckToReload() {
		work.writeStatus("finish_reload", "succ")
		return
	}
	work.writeStatus("prepare_reload", "")

	var stdout string
	versionPath := getParentDir(work.Version, work.Depend)
	bgLoadCmd := "bg_load_base"
	if work.Mode == "delta" {
		bgLoadCmd = "bg_load_patch"
	}
	if work.ActiveVersionList == "" {
		work.ActiveVersionList = "[]"
	}
	for i := 0; i < RELOAD_RETRY_TIMES; i++ {
		reloadCmd := fmt.Sprintf("curl -o /dev/null -s -w %%{http_code} -d '{\"cmd\":\"%s\",\"version_path\":\"/%s\"}' http://127.0.0.1:%s/ControlService/cmd", bgLoadCmd, versionPath, work.Port)
		fmt.Println("reload: ", reloadCmd)
		stdout, _, _ = RetryCmd(reloadCmd, 1)
		fmt.Println("reload stdout: ", stdout)
		if strings.TrimSpace(stdout) == "200" {
			logex.Debugf("bg_load_base return succ")
			break
		} else {
			logex.Warning("bg_load_base return failed")
			time.Sleep(RELOAD_RETRY_INTERVAL_SECOND * time.Second)
		}
	}

	if strings.TrimSpace(stdout) == "200" {
		work.writeStatus("finish_reload", "succ")
	} else {
		work.writeStatus("finish_reload", "failed")
		err = errors.New("reload failed.")
	}

	return
}

func (work *Work) Clear() (err error) {
	work.Service = ""

	var stdout string
	var clearCmd string
	for i := 0; i < RETRY_TIMES; i++ {
		clearCmd = fmt.Sprintf("curl -o /dev/null -s -w %%{http_code} -d '{\"cmd\":\"clear\",\"table_name\":\"%s\"}' http://127.0.0.1:%s/NodeControlService/cmd", work.DictName, work.Port)
		fmt.Println("clear: ", clearCmd)
		stdout, _, _ = RetryCmd(clearCmd, 1)
		fmt.Println("clear stdout: ", stdout)
		if strings.TrimSpace(stdout) == "200" {
			logex.Debugf("clear return succ")
			break
		} else {
			logex.Warning("clear return failed")
			time.Sleep(RELOAD_RETRY_INTERVAL_SECOND * time.Second)
		}
	}

	if strings.TrimSpace(stdout) == "200" {
		err = work.writeStatus("succ", "")
	} else {
		err = work.writeStatus("failed", "")
	}

	return
}

func (work *Work) Check() (err error) {
	if work.Service != "" || work.VersionSign == "" {
		return
	}
	var dataLinkDirs []string
	split := len(strings.Split(work.Source, ";"))
	dataLinkDirs = GetDataLinkDirs(work.DictName, work.Service, work.Version, work.Depend,
		work.DeployPath, work.ShardSeq, split)

	if _, t_err := os.Stat(work.DeployPath); os.IsNotExist(t_err) {
		logex.Noticef("check DeployPath[%s] not exists.", work.DeployPath)
		return
	}

	check_succ := true
	for _, linkDir := range dataLinkDirs {
		parentDir, _ := filepath.Split(linkDir)

		cmd := fmt.Sprintf("ls -l %s | grep %s | awk '{print $9}' | grep -v data | grep -v index", parentDir, work.Depend)

		stdout, _, err := RetryCmd(cmd, RETRY_TIMES)
		if err != nil || stdout == "" {
			check_succ = false
			break
		} else {
			versionList := strings.Split(strings.TrimSpace(stdout), "\n")
			logex.Noticef("calc ver_sign for [%v]", versionList)

			var version_sign string
			var version string
			for i := 0; i < len(versionList); i++ {
				split_index := strings.Index(versionList[i], "_")
				if split_index > 0 && split_index < len(versionList[i]) {
					version = versionList[i][split_index+1:]
				} else {
					version = versionList[i]
				}
				if version_sign == "" {
					version_sign = fmt.Sprintf("%x", md5.Sum([]byte(version)))
				} else {
					version_sign = fmt.Sprintf("%x", md5.Sum([]byte(version_sign)))
				}
			}

			if version_sign != work.VersionSign {
				logex.Warningf("version_sign check failed. real[%v] expect[%v]", version_sign, work.VersionSign)
				check_succ = false
				break
			}
		}
	}

	if !check_succ {
		work.clearPatchData()
		work.clearPatchLink()
		master_host, master_port, _ := GetMaster(work.MasterAddress)
		cmd := fmt.Sprintf("cd %s && export STRATEGY_DIR=%s && ./downloader -h %s -p %s -d %s -s %d",
			work.DeployPath, work.DeployPath, master_host, master_port, work.DictName, work.ShardSeq)
		_, _, err = RetryCmd(cmd, RETRY_TIMES)
	}

	return
}

func (work *Work) Enable() (err error) {
	if work.Port == "" {
		err = errors.New("Enable with invalid port")
		return
	}
	var stdout string
	var cmd string
	versionPath := getParentDir(work.Version, work.Depend)
	for i := 0; i < RELOAD_RETRY_TIMES; i++ {
		if work.Service != "" {
			cmd = fmt.Sprintf("curl -o /dev/null -s -w %%{http_code} -d '{\"cmd\":\"reload_model\",\"version\":\"%s-%s\",\"dict_name\":\"%s\"}' http://127.0.0.1:%s/ControlService/cmd",
				versionPath, work.DictName, work.DictName, work.Port)
		} else {
			cmd = fmt.Sprintf("curl -o /dev/null -s -w %%{http_code} -d '{\"cmd\":\"enable\",\"version\":\"%s\"}' http://127.0.0.1:%s/ControlService/cmd", versionPath, work.Port)
		}
		stdout, _, _ = RetryCmd(cmd, 1)

		if strings.TrimSpace(stdout) == "200" {
			logex.Debugf("enable return succ for %s, work dir is %s", work.Source, work.DeployPath)
			break
		} else {
			logex.Warningf("enable return failed for %s, work dir is %s, error is (%+v)", work.Source, work.DeployPath, err)
			time.Sleep(RELOAD_RETRY_INTERVAL_SECOND * time.Second)
		}
	}

	if strings.TrimSpace(stdout) == "200" {
		err = work.writeStatus("succ", "")
	} else {
		err = work.writeStatus("failed", "")
	}

	if work.Service == "" {
		cmd = fmt.Sprintf("curl -o /dev/null -s -w %%{http_code} -d '{\"cmd\":\"bg_unload\"}' http://127.0.0.1:%s/ControlService/cmd", work.Port)
		stdout, _, _ = RetryCmd(cmd, RETRY_TIMES)
		if strings.TrimSpace(stdout) == "200" {
			logex.Debugf("unload return succ")
		} else {
			logex.Warning("unload return failed")
		}
	}

	RemoveStateFile(work.DictName, work.ShardSeq, work.Service)

	return
}

func (work *Work) Pop() (err error) {
	var stdout string
	var cmd string
	if work.ActiveVersionList == "" {
		work.ActiveVersionList = "[]"
	}
	for i := 0; i < RELOAD_RETRY_TIMES; i++ {
		cmd = fmt.Sprintf("curl -o /dev/null -s -w %%{http_code} -d '{\"cmd\":\"pop\",\"table_name\":\"%s\",\"active_versions\":%v}' http://127.0.0.1:%s/NodeControlService/cmd", work.DictName, work.ActiveVersionList, work.Port)
		fmt.Println("pop: ", cmd)
		stdout, _, _ = RetryCmd(cmd, 1)
		fmt.Println("pop stdout: ", stdout)
		if strings.TrimSpace(stdout) == "200" {
			logex.Debugf("pop return succ")
			break
		} else {
			logex.Warning("pop return failed")
			time.Sleep(RELOAD_RETRY_INTERVAL_SECOND * time.Second)
		}
	}

	if strings.TrimSpace(stdout) == "200" {
		err = work.writeStatus("succ", "")
	} else {
		err = work.writeStatus("failed", "")
	}

	RemoveStateFile(work.DictName, work.ShardSeq, work.Service)
	return
}

func writeStateFile(dictName string, shardSeq int, service, state string) {
	stateFile := fmt.Sprintf(".state_%s_%d", dictName, shardSeq)
	if service != "" {
		stateFile = stateFile + "_" + service
	}

	cmd := fmt.Sprintf("echo '%s' > %s/state/%s", state, Dir, stateFile)
	if _, _, err := RetryCmd(cmd, RETRY_TIMES); err != nil {
		logex.Warningf("%s error (%+v)", cmd, err)
	}
}

func RemoveStateFile(dictName string, shardSeq int, service string) {
	stateFile := fmt.Sprintf(".state_%s_%d", dictName, shardSeq)
	if service != "" {
		stateFile = stateFile + "_" + service
	}

	cmd := fmt.Sprintf("rm -f %s/state/%s", Dir, stateFile)
	if _, _, err := RetryCmd(cmd, RETRY_TIMES); err != nil {
		logex.Warningf("%s error (%+v)", cmd, err)
	}
}

func (work *Work) writeStatus(status string, finishStatus string) (err error) {
	work.Status = status
	work.FinishStatus = finishStatus
	state, _ := json.Marshal(work)
	writeStateFile(work.DictName, work.ShardSeq, work.Service, string(state))
	return
}

func DoDownloadIndividual(source, downloadDir string, isService bool, timeOut int, ch chan error, wg *sync.WaitGroup) {
	err := errors.New("DoDownloadIndividual start")
	for i := 0; i < RETRY_TIMES; i++ {
		err = FtpDownload(source, downloadDir, timeOut)
		if err == nil {
			logex.Debugf("download %s to %s succ", source, downloadDir)
			if !isService {
				err = FtpDownload(source+".md5", downloadDir, timeOut)
			}
		} else {
			logex.Warningf("download error , source %s, downloadDir %s, err (%+v)", source, downloadDir, err)
			continue
		}

		if err == nil && isService {
			// touch download_succ file
			cmd := fmt.Sprintf("touch %s", path.Join(downloadDir, DOWNLOAD_DONE_MARK_FILE))
			RetryCmd(cmd, RETRY_TIMES)
			break
		}

		// download md5 file succ, md5check
		if err == nil {
			// md5sum -c
			fileName := GetFileName(source)
			err = checkMd5(path.Join(downloadDir, fileName), path.Join(downloadDir, fileName+".md5"))
			logex.Warningf("md5sum check %s %s return (%+v)", path.Join(downloadDir, fileName), path.Join(downloadDir, fileName+".md5"), err)
			if err == nil {
				// touch download_succ file
				cmd := fmt.Sprintf("touch %s", path.Join(downloadDir, DOWNLOAD_DONE_MARK_FILE))
				RetryCmd(cmd, RETRY_TIMES)
				logex.Debugf("md5sum ok, source is %s, dir is %s", source, downloadDir)
				break
			} else {
				logex.Warningf("md5sum error, source is %s, dir is %s", source, downloadDir)
				continue
			}
		} else {
			logex.Warningf("download %s return (%+v)", source+".md5", err)
			continue
		}
	}

	ch <- err
	wg.Done()
}

func checkSources(source string) ([]string, error) {
	sources := strings.Split(source, ";")
	for i := 0; i < len(sources); i++ {
		if sources[i] == "" || (!strings.HasPrefix(sources[i], "ftp://") && !strings.HasPrefix(sources[i], "http://")) {
			return sources, errors.New("Invalid sources")
		}
	}
	return sources, nil
}

func DoDownload(dictName, service, version, depend, mode, source, deployPath string,
	shardSeq int) (err error) {
	sources, err := checkSources(source)
	if err != nil {
		logex.Warningf("checkSources %s return (%+v)", source, err)
		return
	}
	downloadDirs, err := GetDownloadDirs(dictName, service, version, depend, deployPath, shardSeq,
		len(sources))
	if err != nil {
		logex.Warningf("GetDownloadDirs %s return (%+v)", source, err)
		return
	}
	version_suffix := ""
	if service != "" {
		version_suffix = version_suffix + "-" + dictName
	}
	if !checkToDownload(downloadDirs) {
		cmd := fmt.Sprintf("cd %s/cube_data && echo %s > VERSION && cp VERSION VERSION-%s",
			deployPath, getParentDir(version, depend)+version_suffix, dictName)
		_, _, err = RetryCmd(cmd, RETRY_TIMES)
		logex.Debugf("echo VERSION cmd:[%s] err:[%v]", cmd, err)
		return
	}

	ch := make(chan error, len(sources))
	wg := sync.WaitGroup{}
	j := 0
	numCo := 0
	for ; j < len(sources); j++ {
		if numCo >= MAX_DOWN_CO {
			wg.Wait()
			logex.Noticef("DoDownload co down.")
			numCo = 0
		}
		numCo += 1
		wg.Add(1)
		time.Sleep(2000 * time.Millisecond)
		timeOut := 900
		if mode == "base" {
			timeOut = 3600
		}
		go DoDownloadIndividual(sources[j], downloadDirs[j], (service != ""), timeOut, ch, &wg)
	}
	wg.Wait()
	close(ch)
	for err = range ch {
		if err != nil {
			return
		}
	}
	cmd := fmt.Sprintf("cd %s/cube_data && echo %s > VERSION && cp VERSION VERSION-%s",
		deployPath, getParentDir(version, depend)+version_suffix, dictName)
	_, _, err = RetryCmd(cmd, RETRY_TIMES)
	logex.Debugf("echo VERSION cmd:[%s] err:[%v]", cmd, err)
	return
}

func FtpDownload(source string, dest string, timeOut int) (err error) {
	dlCmd := fmt.Sprintf("wget --quiet --level=100 -P %s %s --limit-rate=10240k", dest, source)
	fmt.Println(dlCmd)

	_, _, err = RetryCmdWithSleep(dlCmd, RETRY_TIMES)
	return
}

func checkToDownload(downloadDirs []string) bool {
	for _, v := range downloadDirs {
		if _, err := os.Stat(path.Join(v, DOWNLOAD_DONE_MARK_FILE)); err != nil {
			logex.Noticef("check [%v] not exists.", v)
			return true
		}
	}

	return false
}

// simple hash
func getDownloadDisk(dictName string, shardSeq int) string {
	index := len(dictName) * shardSeq % len(disks)

	return disks[index]
}

func getParentDir(version string, depend string) (dir string) {
	if version == depend {
		dir = depend
	} else {
		dir = depend + "_" + version
	}

	return
}

func GetFileName(source string) (fileName string) {
	s := strings.Split(source, "/")
	fileName = s[len(s)-1]

	return
}

func checkMd5(file string, fileMd5 string) (err error) {
	cmd := fmt.Sprintf("md5sum %s | awk '{print $1}'", file)
	stdout, _, _ := pipeline.Run(exec.Command("/bin/sh", "-c", cmd))
	real_md5 := stdout.String()
	cmd = fmt.Sprintf("cat %s | awk '{print $1}'", fileMd5)
	stdout, _, _ = pipeline.Run(exec.Command("/bin/sh", "-c", cmd))
	given_md5 := stdout.String()

	if real_md5 != given_md5 {
		logex.Warningf("checkMd5 failed real_md5[%s] given_md5[%s]", real_md5, given_md5)
		err = errors.New("checkMd5 failed")
	}

	return
}

func RetryCmd(cmd string, retryTimes int) (stdoutStr string, stderrStr string, err error) {
	for i := 0; i < retryTimes; i++ {
		stdout, stderr, e := pipeline.Run(exec.Command("/bin/sh", "-c", cmd))
		stdoutStr = stdout.String()
		stderrStr = stderr.String()
		err = e

		logex.Debugf("cmd %s, stdout: %s, stderr: %s, err: (%+v)", cmd, stdoutStr, stderrStr, err)
		if err == nil {
			break
		}
	}

	return
}

func RetryCmdWithSleep(cmd string, retryTimes int) (stdoutStr string, stderrStr string, err error) {
	for i := 0; i < retryTimes; i++ {
		stdout, stderr, e := pipeline.Run(exec.Command("/bin/sh", "-c", cmd))
		stdoutStr = stdout.String()
		stderrStr = stderr.String()
		err = e

		logex.Debugf("cmd %s, stdout: %s, stderr: %s, err: (%+v)", cmd, stdoutStr, stderrStr, err)
		if err == nil {
			break
		}
		time.Sleep(10000 * time.Millisecond)
	}

	return
}
