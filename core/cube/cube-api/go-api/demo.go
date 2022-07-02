package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
)

func main() {
	dict_name := flag.String("n", "test", "cube name")
	conf_path := flag.String("c", "./conf/cube.conf", "cube conf path")
	input_path := flag.String("i", "./input.json", "keys to seek")
	output_path := flag.String("o", "./output.json", "result to save")
	flag.Parse()
	bytes, err := ioutil.ReadFile(*conf_path)
	if err != nil {
		fmt.Println("读取配置文件失败", err)
		return
	}
	var meta Meta
	err = json.Unmarshal(bytes, &meta.Servers)
	if err != nil {
		fmt.Println("解析数据失败", err)
		return
	}

	err = meta.Seek(*dict_name, *input_path, *output_path)
	if err != nil {
		fmt.Println(err)
	}
	return
}
