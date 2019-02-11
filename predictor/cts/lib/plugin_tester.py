#!/usr/bin/env python
# -*- coding:gbk -*-
"""
注册类：RegxxxConfData，RegxxxReq，RegxxxXbox，RegxxxAd，xxx为组件名
"""
from lib.pluginlib.plugin_common import ConfData
from lib.pluginlib.plugin_common import TreeConfData
from lib.pluginlib.plugin_common import CommonIndex


class RegpredictorConfData(object):
    """
        注册wtitleq组件的conf和data文件
    """
    def __init__(self, path):
        self.path = path
        self.conf = {}
        self.data = {}
        self.conf['ub'] = ConfData(path=self.path + "/conf/ub.conf", connect_flag=":")
        self.data['lr_model'] = CommonIndex(path=self.path + \
                                '/data/lr-model/wtitleq_model_file.sign', 
                                col_list=['key', 'value'], 
                                format='B')


class RegpredictorReq(object):
    """
        注册wtitleq组件的默认请求
    """
    def __init__(self):
        self.plugin_term = {}
        cmd_tag = 'cmd_tag0'
        query_schema_list = []
        query_value_list = []
        pair_schema_list = ['query', 
                            'wadptid', 
                            'wbwsid', 
                            'omit_buf', 
                            'title', 
                            'desc', 
                            'cmatch', 
                            'bidword', 
                            'dynamic_new_title']
        pair_value_list = ['鲜花', 
                           '0', 
                           '3', 
                           '鲜花', 
                           '鲜花%2C本地实体鲜花店100%25保证%21', 
                           '鲜花品质100%25%2C主城最快2小时送到%2C全天24时在线订花%21市区内免费送花上门%21鲜%2E%2E', 
                           '223', 
                           '鲜花', 
                           '美丽鲜花']
        cmd_str = '/titleq/wise/ctr'
        req_term = {"query_schema": query_schema_list,
                "pair_schema": pair_schema_list,
                "query_value": query_value_list,
                "pair_value": pair_value_list,
                "cmd": cmd_str}
        self.plugin_term.update({cmd_tag: req_term})
        self.plugin_list = self.plugin_term.keys()


class RegpredictorNewXbox(object):
    """
        注册wtitleq组件的xbox
    """
    def __init__(self):
        self.need_xbox = True
        self.stub_conf = 'xboxstub.conf'
        self.stub_name = 'xboxstub'
        self.conf_list = ['xbox-wtitleq_pegasus.conf']


class RegpredictorAd(object):
    """
        注册wtitleq组件是否需要构造广告库
    """
    def __init__(self):
        self.need_adstub = False


