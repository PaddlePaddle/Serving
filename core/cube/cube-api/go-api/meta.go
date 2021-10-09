package main

import "fmt"

type Meta struct {
	Servers []CubeServer `json:"servers,omitempty"`
}

func (meta *Meta) Seek(dict_name string, input string, output string) (err error) {
	var server CubeServer

	for _, s := range meta.Servers {
		if s.Name == dict_name {
			server = s
			break
		}
	}
	if server.Name != dict_name {
		err = fmt.Errorf("%s server not exist", dict_name)
		return err
	}
	err = server.Seek(input, output)
	return err
}
