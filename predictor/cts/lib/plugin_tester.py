#!/usr/bin/env python
# -*- coding:gbk -*-
"""
ע���ࣺRegxxxConfData��RegxxxReq��RegxxxXbox��RegxxxAd��xxxΪ�����
"""
from lib.pluginlib.plugin_common import ConfData
from lib.pluginlib.plugin_common import TreeConfData
from lib.pluginlib.plugin_common import CommonIndex


class RegpredictorConfData(object):
    """
        ע��wtitleq�����conf��data�ļ�
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
        ע��wtitleq�����Ĭ������
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
        pair_value_list = ['�ʻ�', 
                           '0', 
                           '3', 
                           '�ʻ�', 
                           '�ʻ�%2C����ʵ���ʻ���100%25��֤%21', 
                           '�ʻ�Ʒ��100%25%2C�������2Сʱ�͵�%2Cȫ��24ʱ���߶���%21����������ͻ�����%21��%2E%2E', 
                           '223', 
                           '�ʻ�', 
                           '�����ʻ�']
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
        ע��wtitleq�����xbox
    """
    def __init__(self):
        self.need_xbox = True
        self.stub_conf = 'xboxstub.conf'
        self.stub_name = 'xboxstub'
        self.conf_list = ['xbox-wtitleq_pegasus.conf']


class RegpredictorAd(object):
    """
        ע��wtitleq����Ƿ���Ҫ�������
    """
    def __init__(self):
        self.need_adstub = False


