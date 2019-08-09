//-*- coding:utf-8; indent-tabs-mode:nil; -*-
package agent

import (
	"fmt"
	"encoding/json"
	"github.com/Badangel/logex"
)

func initPostHandlers() {
	postHandler = map[string]handlerFunc{
		"/agent/cmd":           PostCmd,
	}
}

func PostCmd(subpath string, m map[string]string, b []byte) (string, string, error) {
	var work Work
	err := json.Unmarshal(b, &work)
	if err != nil {
		logex.Warningf("Unmarshal from %s error (+%v)", string(b), err)
		return quote(""), "", fmt.Errorf("Work json unmarshal work failed, %v", err)
	}

	if _, ok := CmdWorkFilter.Load(work.Token()); ok {
		logex.Warningf("Another work with same token is doing. Token(%s)", work.Token())
		return quote(""), "", fmt.Errorf("Another work with same key is doing.", err)
	}
	
	CmdWorkFilter.Store(work.Token(), true)
	err = work.DoWork()
	CmdWorkFilter.Delete(work.Token())
	if err != nil {
		return quote(""), "", fmt.Errorf("Do work failed.", err)
	}

	return quote(""), "", err
}