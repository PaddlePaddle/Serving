package main

import (
	"bufio"
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
)

type Input struct {
	Keys []uint64 `json:"keys"`
}

type SingleValue struct {
	Status uint32 `json:"status"`
	Value  string `json:"value"`
}
type Output struct {
	Values []SingleValue `json:"values"`
}

type ServerNode struct {
	Ip   string `json:"ip"`
	Port uint64 `json:"port"`
}

type CubeServer struct {
	Name  string       `json:"dict_name"`
	Shard uint64       `json:"shard"`
	Nodes []ServerNode `json:"nodes"`
}

func (server *CubeServer) SplitKeys(keys []uint64) (splited_keys map[uint64]Input, offset map[uint64][]uint64) {
	splited_keys = make(map[uint64]Input)

	offset = make(map[uint64][]uint64)
	for i, key := range keys {
		shard_id := key % server.Shard
		temp_split, _ := splited_keys[shard_id]
		temp_split.Keys = append(temp_split.Keys, key)
		splited_keys[shard_id] = temp_split

		temp_offset, _ := offset[shard_id]
		temp_offset = append(temp_offset, uint64(i))
		offset[shard_id] = temp_offset
	}

	return splited_keys, offset
}

func (server *CubeServer) Seek(input string, output_path string) (err error) {
	file, err := os.Open(input)
	if err != nil {
		return err
	}
	defer file.Close()

	buf := bufio.NewReader(file)

	for {
		line, err := buf.ReadBytes('\n')
		//line = strings.TrimSpace(line)
		if err != nil || io.EOF == err {
			break
		}
		var temp_input Input
		json.Unmarshal(line, &temp_input)
		key_nums := len(temp_input.Keys)
		var output Output
		output.Values = make([]SingleValue, key_nums+1)
		splited_keys, offset := server.SplitKeys(temp_input.Keys)
		for shard_id, keys := range splited_keys {
			cur_output, _ := server.Post(shard_id, keys)
			for index, single_value := range cur_output.Values {
				output.Values[offset[shard_id][index]] = single_value
			}
		}
		json_str, _ := json.Marshal(output)
		fp, err := os.OpenFile(output_path, os.O_RDWR|os.O_APPEND|os.O_CREATE, 0755)
		if err != nil {
			log.Fatal(err)
		}
		defer fp.Close()
		_, err = fp.Write(json_str)
	}
	return err
}

func (server *CubeServer) Post(shard_id uint64, input Input) (output Output, err error) {
	if shard_id >= uint64(len(server.Nodes)) {
		err = fmt.Errorf("have no shard:%v", shard_id)
		return output, err
	}
	json_str, _ := json.Marshal(input)
	URL := fmt.Sprintf("http://%s:%v/DictService/seek", server.Nodes[shard_id].Ip, server.Nodes[shard_id].Port)
	req, err := http.NewRequest("POST", URL, bytes.NewBuffer(json_str))
	if err != nil {
		return output, err
	}
	req.Header.Set("Content-Type", "application/json")
	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		return output, err
	}
	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return output, err
	}
	err = json.Unmarshal(body, &output)
	return output, err
}
