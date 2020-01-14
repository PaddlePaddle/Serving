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
	"errors"
	"fmt"
	"io/ioutil"
	"net/http"
	"net/url"
	"strconv"
	"strings"
	"time"

	"github.com/Badangel/logex"
)

type handlerFunc func(subpath string, m map[string]string, b []byte) (string, string, error)

var ( // key = subpath; eg: path="/checker/job", key="job"
	getHandler    map[string]handlerFunc
	putHandler    map[string]handlerFunc
	deleteHandler map[string]handlerFunc
	postHandler   map[string]handlerFunc
)

func StartHttp(addr string) error {

	// init handlers:
	initGetHandlers()
	initPostHandlers()

	http.HandleFunc("/agent/", handleRest)
	logex.Notice("start http ", addr)
	return http.ListenAndServe(addr, nil)
}

func handleRest(w http.ResponseWriter, r *http.Request) {
	var (
		req_log string
		status  int32
	)
	time_begin := time.Now()

	cont_type := make([]string, 1, 1)
	cont_type[0] = "application/json"
	header := w.Header()
	header["Content-Type"] = cont_type
	w.Header().Add("Access-Control-Allow-Origin", "*")

	m := parseHttpKv(r)
	b, _ := ioutil.ReadAll(r.Body)

	req_log = fmt.Sprintf("handle %v %v %v from %v, len(m)=%v, m=%+v",
		r.Method, r.URL.Path, r.URL.RawQuery, r.RemoteAddr, len(m), m)

	api := r.URL.Path

	var showHandler map[string]handlerFunc
	switch r.Method {
	case "GET":
		showHandler = getHandler
	case "POST": // create
		showHandler = postHandler
	case "PUT": // update
		showHandler = putHandler
	case "DELETE":
		showHandler = deleteHandler
	default:
		logex.Warningf(`{"error":1, "message":"unsupport method %v"}`, r.Method)
	}

	handler, ok := showHandler[api]

	if !ok {
		key_list := make([]string, 0, len(showHandler))
		for key := range showHandler {
			key_list = append(key_list, key)
		}
		status = 2
		fmt.Fprintf(w, `{"success":"%v", "message":"wrong api", "method":"%s", "api":"%s", "api_list":"%v"}`,
			status, r.Method, api, key_list)

		logex.Noticef(`%v, time=%v, status=%v`,
			req_log, time.Now().Sub(time_begin).Nanoseconds()/1000000, status)
		return
	}

	var s string
	rst, handle_log, err := handler(api, m, b)
	if err == nil {
		status = 0
		s = fmt.Sprintf(`{"success":"%v", "message":"query ok", "data":%s}`, status, rst)
	} else {
		status = 255
		s = fmt.Sprintf(`{"success":"%v", "message":%v, "data":%s}`,
			status, quote(err.Error()), rst)
	}

	if isJsonDict(s) {
		fmt.Fprintln(w, s)
	} else {
		logex.Fatalf("invalid json: %v", s)
	}

	if err == nil {
		logex.Noticef(`%v, time=%v, status=%v, handle_log=%v`,
			req_log, time.Now().Sub(time_begin).Nanoseconds()/1000000,
			status, quote(handle_log))
	} else {
		logex.Noticef(`%v, time=%v, status=%v, err=%v, handle_log=%v`,
			req_log, time.Now().Sub(time_begin).Nanoseconds()/1000000,
			status, quote(err.Error()), quote(handle_log))
	}
}

func parseHttpKv(r *http.Request) map[string]string {
	r.ParseForm()
	m := make(map[string]string)
	for k, v := range r.Form {
		switch k {
		case "user": // remove @baidu.com for user
			m[k] = strings.Split(v[0], "@")[0]
		default:
			m[k] = v[0]
		}
	}

	// allow passing hostname for debug
	if _, ok := m["hostname"]; !ok {
		ip := r.RemoteAddr[:strings.Index(r.RemoteAddr, ":")]
		m["hostname"], _ = getHostname(ip)
	}
	return m
}

// restReq sends a restful request to requrl and returns response body.
func restReq(method, requrl string, timeout int, kv *map[string]string) (string, error) {
	logex.Debug("####restReq####")
	logex.Debug(*kv)
	data := url.Values{}
	if kv != nil {
		for k, v := range *kv {
			logex.Trace("req set:", k, v)
			data.Set(k, v)
		}
	}
	if method == "GET" || method == "DELETE" {
		requrl = requrl + "?" + data.Encode()
		data = url.Values{}
	}

	logex.Notice(method, requrl)
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
