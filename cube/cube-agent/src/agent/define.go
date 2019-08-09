package agent

import (
	"errors"
	_ "github.com/Badangel/logex"
	"github.com/Badangel/pipeline"
	"os/exec"
	"strconv"
	"strings"
	"sync"
)

var (
	Dir           string
	WorkerNum     int
	QueueCapacity int32
	MasterHost    []string
	MasterPort    []string

	TestHostname string
	TestIdc      string
	ShardLock    sync.RWMutex

	CmdWorkPool   *WorkPool
	CmdWorkFilter sync.Map
)

type (
	Status struct {
		Status  string `json:"status"`
		Version string `json:"version"`
	}

	MasterResp struct {
		Success string `json:"success"`
		Message string `json:"message"`
		Data    string `json:"data"`
	}

	ShardInfo struct {
		DictName   string
		ShardSeq   int
		SlotIdList string
		DataDir    string
		Service    string `json:"service,omitempty"`
		Libcube    string `json:"libcube,omitempty"`
	}

	CubeResp struct {
		Status     int    `json:"status"`
		CurVersion string `json:"cur_version"`
		BgVersion  string `json:"bg_version"`
	}
)

var BUILTIN_STATUS = Status{"RUNNING", "3.0.0.1"}

var ShardInfoMap map[string]map[string]*ShardInfo
var disks []string

func GetMaster(master string) (host, port string, err error) {
	if len(ShardInfoMap) < 1 {
		return "", "", errors.New("empty master list.")
	}
	if master == "" {
		return MasterHost[0], MasterPort[0], nil
	}
	if _, ok := ShardInfoMap[master]; ok {
		m := strings.Split(master, ":")
		if len(m) != 2 {
			return MasterHost[0], MasterPort[0], nil
		}
		return m[0], m[1], nil
	} else {
		return MasterHost[0], MasterPort[0], nil
	}
}

func init() {
	dfCmd := "df -h | grep -E '/home|/ssd'"
	stdout, _, err := pipeline.Run(exec.Command("/bin/bash", "-c", dfCmd))

	if err == nil && stdout.String() != "" {
		t := strings.TrimSpace(stdout.String())
		diskLi := strings.Split(t, "\n")
		for _, diskStr := range diskLi {
			disk := strings.Fields(diskStr)
			usedPercent, _ := strconv.Atoi(strings.TrimRight(disk[4], "%"))
			if usedPercent <= 40 {
				disks = append(disks, disk[5])
			}
		}
	}

	if len(disks) == 0 {
		disks = append(disks, "/home")
	}

	//logex.Debugf("available disks found: (%+v)", disks)
}
