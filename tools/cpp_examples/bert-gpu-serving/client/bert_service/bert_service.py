# coding:utf-8
import sys
import numpy as np
import paddlehub as hub
import ujson
import random
from paddlehub.common.logger import logger
import socket

_ver = sys.version_info
is_py2 = (_ver[0] == 2)
is_py3 = (_ver[0] == 3)

if is_py2:
    import httplib
if is_py3:
    import http.client as httplib


class BertService():
    def __init__(self,
                 profile=False,
                 max_seq_len=128,
                 model_name="bert_uncased_L-12_H-768_A-12",
                 show_ids=False,
                 do_lower_case=True,
                 process_id=0,
                 retry=3,
                 load_balance='round_robin'):
        self.process_id = process_id
        self.reader_flag = False
        self.batch_size = 0
        self.max_seq_len = max_seq_len
        self.profile = profile
        self.model_name = model_name
        self.show_ids = show_ids
        self.do_lower_case = do_lower_case
        self.con_list = []
        self.con_index = 0
        self.load_balance = load_balance
        self.server_list = []
        self.serving_list = []
        self.feed_var_names = ''
        self.retry = retry

        module = hub.Module(name=self.model_name)
        inputs, outputs, program = module.context(
            trainable=True, max_seq_len=self.max_seq_len)
        input_ids = inputs["input_ids"]
        position_ids = inputs["position_ids"]
        segment_ids = inputs["segment_ids"]
        input_mask = inputs["input_mask"]
        self.feed_var_names = input_ids.name + ';' + position_ids.name + ';' + segment_ids.name + ';' + input_mask.name
        self.reader = hub.reader.ClassifyReader(
            vocab_path=module.get_vocab_path(),
            dataset=None,
            max_seq_len=self.max_seq_len,
            do_lower_case=self.do_lower_case)
        self.reader_flag = True

    def add_server(self, server='127.0.0.1:8010'):
        self.server_list.append(server)
        self.check_server()

    def add_server_list(self, server_list):
        for server_str in server_list:
            self.server_list.append(server_str)
        self.check_server()

    def check_server(self):
        for server in self.server_list:
            client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server_ip = server.split(':')[0]
            server_port = int(server.split(':')[1])
            client.connect((server_ip, server_port))
            client.send(b'pending server')
            response = client.recv(1024).decode()

            response_list = response.split('\t')
            status_code = int(response_list[0].split(':')[1])

            if status_code == 0:
                server_model = response_list[1].split(':')[1]
                if server_model == self.model_name:
                    serving_port = response_list[2].split(':')[1]
                    serving_ip = server_ip
                    self.serving_list.append(serving_ip + ':' + serving_port)
                else:
                    logger.error('model_name not match, server {}  using : {} '.
                                 format(server, server_model))
            else:
                error_msg = response_list[1]
                logger.error('connect server {} failed. {}'.format(server,
                                                                   error_msg))

    def request_server(self, request_msg):
        if self.load_balance == 'round_robin':
            try:
                cur_con = httplib.HTTPConnection(self.serving_list[
                    self.con_index])
                cur_con.request('POST', "/BertService/inference", request_msg,
                                {"Content-Type": "application/json"})
                response = cur_con.getresponse()
                response_msg = response.read()
                response_msg = ujson.loads(response_msg)
                self.con_index += 1
                self.con_index = self.con_index % len(self.serving_list)
                return response_msg

            except BaseException as err:
                logger.warning("Infer Error with server {} : {}".format(
                    self.serving_list[self.con_index], err))
                if len(self.serving_list) == 0:
                    logger.error('All server failed, process will exit')
                    return 'fail'
                else:
                    self.con_index += 1
                    return 'retry'

        elif self.load_balance == 'random':
            try:
                random.seed()
                self.con_index = random.randint(0, len(self.serving_list) - 1)
                logger.info(self.con_index)
                cur_con = httplib.HTTPConnection(self.serving_list[
                    self.con_index])
                cur_con.request('POST', "/BertService/inference", request_msg,
                                {"Content-Type": "application/json"})
                response = cur_con.getresponse()
                response_msg = response.read()
                response_msg = ujson.loads(response_msg)

                return response_msg
            except BaseException as err:

                logger.warning("Infer Error with server {} : {}".format(
                    self.serving_list[self.con_index], err))
                if len(self.serving_list) == 0:
                    logger.error('All server failed, process will exit')
                    return 'fail'
                else:
                    self.con_index = random.randint(0,
                                                    len(self.serving_list) - 1)
                    return 'retry'

        elif self.load_balance == 'bind':

            try:
                self.con_index = int(self.process_id) % len(self.serving_list)
                cur_con = httplib.HTTPConnection(self.serving_list[
                    self.con_index])
                cur_con.request('POST', "/BertService/inference", request_msg,
                                {"Content-Type": "application/json"})
                response = cur_con.getresponse()
                response_msg = response.read()
                response_msg = ujson.loads(response_msg)

                return response_msg
            except BaseException as err:

                logger.warning("Infer Error with server {} : {}".format(
                    self.serving_list[self.con_index], err))
                if len(self.serving_list) == 0:
                    logger.error('All server failed, process will exit')
                    return 'fail'
                else:
                    self.con_index = int(self.process_id) % len(
                        self.serving_list)
                    return 'retry'

    def prepare_data(self, text):
        self.batch_size = len(text)
        data_generator = self.reader.data_generator(
            batch_size=self.batch_size, phase='predict', data=text)
        result = []
        for run_step, batch in enumerate(data_generator(), start=1):
            request = []
            token_list = batch[0][0].reshape(-1).tolist()
            pos_list = batch[0][1].reshape(-1).tolist()
            sent_list = batch[0][2].reshape(-1).tolist()
            mask_list = batch[0][3].reshape(-1).tolist()
            for si in range(self.batch_size):
                instance_dict = {}
                instance_dict["token_ids"] = token_list[si * self.max_seq_len:(
                    si + 1) * self.max_seq_len]
                instance_dict["sentence_type_ids"] = sent_list[
                    si * self.max_seq_len:(si + 1) * self.max_seq_len]
                instance_dict["position_ids"] = pos_list[si * self.max_seq_len:(
                    si + 1) * self.max_seq_len]
                instance_dict["input_masks"] = mask_list[si * self.max_seq_len:(
                    si + 1) * self.max_seq_len]
                request.append(instance_dict)

            request = {"instances": request}
            request["max_seq_len"] = self.max_seq_len
            request["feed_var_names"] = self.feed_var_names
            request_msg = ujson.dumps(request)
            if self.show_ids:
                logger.info(request_msg)

        return request_msg

    def encode(self, text):
        if len(self.serving_list) == 0:
            logger.error('No match server.')
            return -1
        if type(text) != list:
            raise TypeError('Only support list')
        request_msg = self.prepare_data(text)

        response_msg = self.request_server(request_msg)
        retry = 0
        while type(response_msg) == str and response_msg == 'retry':
            if retry < self.retry:
                retry += 1
                logger.info('Try to connect another servers')
                response_msg = self.request_server(request_msg)
            else:
                logger.error('Request failed after {} times retry'.format(
                    self.retry))
                break
        retry = 0
        result = []
        for msg in response_msg["instances"]:
            for sample in msg["instances"]:
                result.append(sample["values"])

        #request end
        return result


def test():

    bc = BertService(
        model_name='bert_chinese_L-12_H-768_A-12',
        max_seq_len=20,
        show_ids=False,
        do_lower_case=True)
    bc.add_server('127.0.0.1:8010')
    result = bc.encode([["远上寒山石径斜"], ])
    print(result[0])


if __name__ == '__main__':
    test()
