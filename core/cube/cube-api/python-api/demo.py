#coding=utf-8

import requests
import sys
import json

class Meta(object):
    """记录cube分片server路由"""
    def __init__(self, conf_path):
        """根据配置文件初始化路由"""
        self.server_api = "/DictService/seek"
        self.server_meta = {}
        with open(conf_path, "r", encoding="utf8") as fp:
            cube_servcers = json.load(fp)
            for server in cube_servcers:
                self.server_meta[server["dict_name"]] = server
            fp.close()

    def seek(self, dict_name, keys_path, save_path):
        """查询"""
        save_file = open(save_path, 'w')
        with open(keys_path, "r", encoding="utf8") as fp:
            lines = fp.readlines()
            for line in lines:
                json_line = json.loads(line)
                values = [{} for i in range(len(json_line["keys"]))]
                splited_keys, offset = self.split_keys(json_line)
                for shard_id, keys in splited_keys.items():
                    results = self.post(dict_name, shard_id, keys)
                    for i, result in enumerate(results["values"]):
                        values[offset[shard_id][i]] = result
                cur_line_results = {}
                cur_line_results["values"] = values
                
                json.dump(cur_line_results, save_file)
                save_file.write("\n")
                
            fp.close()
        save_file.close()

    def split_keys(self, json_line):
        """根据key值及分片数判断去哪一个分片上查询"""
        keys_split = {}
        offset = {}
        i = 0
        for key in json_line["keys"]:
            shard_id = key % self.server_meta[dict_name]["shard"]
            if shard_id not in keys_split:
                keys_split[shard_id] = []
            keys_split[shard_id].append(key)
            if shard_id not in offset:
                offset[shard_id] = []
            offset[shard_id].append(i)
            i += 1
        return keys_split, offset

    def post(self, dict_name, shard_id, keys):
        """向分片server发送post请求"""
        api = "http://%s:%s%s" % (self.server_meta[dict_name]["nodes"][shard_id]["ip"],
            self.server_meta[dict_name]["nodes"][shard_id]["port"],
            self.server_api)
        data = {"keys": keys}
        response = requests.post(api, json.dumps(data))
        return response.json()


if __name__ == '__main__':
    if len(sys.argv) != 5:
        print('please usage: python demo.py conf_path dict_name keys_path save_path')
        exit(0)
    conf_path = sys.argv[1]
    dict_name = sys.argv[2]
    keys_path = sys.argv[3]
    save_path = sys.argv[4]
    meta = Meta(conf_path)
    meta.seek(dict_name, keys_path, save_path)
