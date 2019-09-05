# -*- coding: utf-8 -*-
import json
import urllib2

data = {"a": 1, "b": 0.5}
request_json = json.dumps(data)

req = urllib2.Request("http://127.0.0.1:8010/BuiltinTestEchoService/inference")
req.add_header('Content-Type',
               'application/json')  # 如果server版本在r31987后，不需要设置这个。
try:
    response = urllib2.urlopen(req, request_json, 1)
    print response.read()
except urllib2.HTTPError as e:
    print e.reason
