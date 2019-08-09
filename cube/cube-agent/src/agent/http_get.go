//-*- coding:utf-8; indent-tabs-mode:nil; -*-
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
