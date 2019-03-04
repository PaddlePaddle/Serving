# Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""
This module provide http requests demo in json/proto/image sceneras.

Authors: wanlijin(wanlijin01@baidu.com)
Date:    2018/06/26 11:47:46
"""

import sys
import json
import urllib
import urllib2
import proto.xrecord_format_pb2 as xrecord_format_pb2
import proto.int64tensor_service_pb2 as int64tensor_service_pb2
import proto.image_classification_pb2 as image_classification_pb2
import proto.image_class_pb2 as image_class_pb2
from google.protobuf.text_format import MessageToString
from google.protobuf.text_format import Merge

url = 'http://127.0.0.1:8010'

#url = 'http://10.88.157.15:8015'
#url = 'http://10.88.158.21:8010'
#url = 'http://st01-rdqa-dev055-wanlijin01.epc.baidu.com:8010'


def http_json():
    """send json format"""
    reqdata = '{"instances": [{"data": [15, 160, 140, 230, 32, 128], \
            "shape": [2, 3]}, {"data": [1, 2, 3], "shape": [3, 1]}]}'

    requrl = '%s/BuiltinFluidService/inference' % url
    return reqdata, requrl, 'json'


def parse_json(res, method):
    """parse json format"""
    return res


def http_proto():
    """send pb binary format"""
    reqproto = int64tensor_service_pb2.Request()
    ins = reqproto.instances.add()
    ins.data.extend([15, 160, 140, 230, 32, 128])
    ins.shape.extend([2, 3])
    ins = reqproto.instances.add()
    ins.data.extend([1, 2, 3])
    ins.shape.extend([3, 1])
    requrl = '%s/BuiltinFluidService/inference' % url
    return reqproto.SerializeToString(), requrl, 'proto'


def parse_proto(res, method):
    """parse pb binary format"""
    if method == 'debug':
        return res
    resproto = int64tensor_service_pb2.Response()
    resproto.ParseFromString(res)
    return MessageToString(resproto, False, True)


def http_image():
    """send image binary format"""
    file_name = './images/what.jpg'
    with open(file_name, 'rb') as fp:
        payload = fp.read()

    reqimage = image_class_pb2.Request()
    ins = reqimage.instances.add()
    ins.image_binary = payload
    ins.image_length = len(payload)
    requrl = '%s/ClassService/inference' % url
    return reqimage.SerializeToString(), requrl, 'proto'


def parse_image(res, method):
    """parse pb response format(image)"""
    file_name = './images/groundtruth.txt'

    if method == 'debug':
        return res

    with open(file_name, 'r') as fp:
        gt = map(lambda x: x.strip().split(',')[1],
                 fp.read().strip().split('\n'))

    resimage = image_class_pb2.Response()
    resimage.ParseFromString(res)
    ximage = resimage.predictions[0]
    categories = json.loads(ximage.response_json)['categories']
    maxprop = max(categories)
    return "Classify result: %s, reliability: %f" % (gt[categories.index(maxprop)], \
            maxprop)


def send_http(content_type):
    """send&recv runtine"""
    reqdata, requrl, type = getattr(sys.modules[__name__],
                                    "http_%s" % content_type)()
    req = urllib2.Request(
        url=requrl,
        data=reqdata,
        headers={"Content-Type": 'application/%s' % type})

    res_data = urllib2.urlopen(req)
    method = requrl.split('/')[-1]
    print getattr(sys.modules[__name__],
                  "parse_%s" % content_type)(res_data.read(), method)


if __name__ == '__main__':
    """entrence"""
    if len(sys.argv) != 2:
        raise Exception("Invalid args, python http_req content_type")
    ctype = sys.argv[1]
    if ctype not in ['json', 'proto', 'image']:
        raise Exception("Invalid content type, must be:[json, proto, image]")
    send_http(ctype)
