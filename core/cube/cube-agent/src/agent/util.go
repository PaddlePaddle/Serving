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
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"net/url"
	"strconv"
	"strings"
	"time"

	"github.com/Badangel/logex"
)

// restReq sends a restful request to requrl and returns response body.
func RestReq(method, requrl string, timeout int, kv *map[string]string) (string, error) {
	data := url.Values{}
	if kv != nil {
		for k, v := range *kv {
			//logex.Trace("req set:", k, v)
			data.Set(k, v)
		}
	}
	if method == "GET" || method == "DELETE" {
		requrl = requrl + "?" + data.Encode()
		data = url.Values{}
	}

	//logex.Notice(method, requrl)
	req, err := http.NewRequest(method, requrl, bytes.NewBufferString(data.Encode()))
	if err != nil {
		logex.Warning("NewRequest failed:", err)
		return "", err
	}
	if method == "POST" || method == "PUT" {
		req.Header.Add("Content-Type", "application/x-www-form-urlencoded")
		req.Header.Add("Content-Length", strconv.Itoa(len(data.Encode())))
	}

	client := &http.Client{}
	client.Timeout = time.Duration(timeout) * time.Second
	resp, err := client.Do(req)
	if err != nil {
		logex.Warning("Do failed:", err)
		return "", err
	}
	if resp.StatusCode < 200 || resp.StatusCode > 299 {
		logex.Warning("resp status: " + resp.Status)
		return "", errors.New("resp status: " + resp.Status)
	}

	body, err := ioutil.ReadAll(resp.Body)
	return string(body), err
}

// restReq sends a restful request to requrl and returns response body as json.
func JsonReq(method, requrl string, timeout int, kv *map[string]string,
	out interface{}) error {
	s, err := RestReq(method, requrl, timeout, kv)
	logex.Debugf("json request method:[%v], requrl:[%s], timeout:[%v], map[%v], out_str:[%s]", method, requrl, timeout, kv, s)
	if err != nil {
		return err
	}
	return json.Unmarshal([]byte(s), out)
}

func GetHdfsMeta(src string) (master, ugi, path string, err error) {
	ugiBegin := strings.Index(src, "//")
	ugiPos := strings.LastIndex(src, "@")
	if ugiPos != -1 && ugiBegin != -1 {
		ugi = src[ugiBegin+2 : ugiPos]
	}
	src1 := strings.Replace(strings.Replace(src, "hdfs://", "", 1), ugi, "", 1)
	if ugi != "" {
		src1 = src1[1:]
	}
	pos := strings.Index(src1, "/")
	if pos != -1 {
		master = src1[0:pos]
		path = src1[pos:]
	} else {
		logex.Warningf("failed to get the master or path for (%s)", src)
		err = errors.New("invalid master or path found")
	}
	logex.Debugf("parse the (%s) succ, master is %s, ugi is (%s), path is %s", src, master, ugi, path)
	return
}

func getHostIp() (string, error) {
	if addrs, err := net.InterfaceAddrs(); err == nil {
		for _, addr := range addrs {
			ips := addr.String()
			logex.Debugf("get host ip: %v", ips)
			if strings.HasPrefix(ips, "127") {
				continue
			} else {
				list := strings.Split(ips, "/")
				if len(list) != 2 {
					continue
				}
				return list[0], nil
			}
		}
	}
	return "unkown ip", errors.New("get host ip failed")
}

func getHostname(ip string) (hostname string, err error) {
	if hostnames, err := net.LookupAddr(ip); err != nil {
		hostname = ip
		//logex.Warningf("cannot find the hostname of ip (%s), error (%v)", ip, err)
	} else {
		if len(hostnames) > 0 {
			hostname = hostnames[0]
		} else {
			hostname = ip
		}
	}

	return hostname, err
}

func GetLocalHostname() (hostname string, err error) {
	if ip, err := getHostIp(); err == nil {
		return getHostname(ip)
	} else {
		return "unkown ip", err
	}
}

func GetLocalHostnameCmd() (hostname string, err error) {
	cmd := "hostname"
	stdout, _, err := RetryCmd(cmd, RETRY_TIMES)
	if stdout != "" && err == nil {
		hostname := strings.TrimSpace(stdout)
		index := strings.LastIndex(hostname, ".baidu.com")
		if index > 0 {
			return hostname[:strings.LastIndex(hostname, ".baidu.com")], nil
		} else {
			return hostname, nil
		}
	} else {
		logex.Debugf("using hostname cmd failed. err:%v", err)
		return GetLocalHostname()
	}
}

// quote quotes string for json output. eg: s="123", quote(s)=`"123"`
func quote(s string) string {
	return fmt.Sprintf("%q", s)
}

// quoteb quotes byte array for json output.
func quoteb(b []byte) string {
	return quote(string(b))
}

// quotea quotes string array for json output
func quotea(a []string) string {
	b, _ := json.Marshal(a)
	return string(b)
}

func isJsonDict(s string) bool {
	var js map[string]interface{}
	return json.Unmarshal([]byte(s), &js) == nil
}
