#!/usr/bin/env python
# -*- coding:gbk -*-
"""
case created by templete
"""
import sys
sys.path.append(r'./lib/protolib')
print("sys path is : %s " % str(sys.path))

import os
import json
import commands
from lib.protolib.dense_service_pb2 import Request
from lib.protolib.dense_service_pb2 import Response
from lib.pluginlib.plugin_util import Util as ut

from lib.pluginlib.plugin_case import PluginCase
from lib.pluginlib.plugin_module import PluginModule
from lib.pluginlib.plugin_apistub import ApiStub

class TestDenseService(PluginCase):
    """test wtitleq case class"""
    OWNER="zhangwenbo03"
    quick=['ALL']
    low=[]
    daily=[]
    ignorelist=[]
    RESTART=True

    def setUp(self):
        """setup something before run case"""
        pass

    def tearDown(self):
        """tear down after run case"""
        self.t.stop()
        print "stop finished"
        pass

    def testDemoCase(self):
        """demo case"""
        req = Request()
        denseIns = req.instances.add()
        denseIns.features.append(10)
        denseIns.features.append(13)
        denseIns.features.append(200)

        service = "BuiltinDenseFormatService"
        type = "debug"

        ut_obj = ut()  
        dict_val = ut_obj.pb2dict(req)
        json_val = ut_obj.dict2json(dict_val)

        self.t.restart()
        self.t.tester.sendJsonData(json_val, service, type)

        print "execute demo case"

