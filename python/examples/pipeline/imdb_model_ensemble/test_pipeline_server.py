# Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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
# pylint: disable=doc-string-missing
import numpy as np
from paddle_serving_app.reader.imdb_reader import IMDBDataset
import logging
from paddle_serving_server.web_service import WebService
from paddle_serving_server.pipeline import Op, RequestOp, ResponseOp
from paddle_serving_server.pipeline import PipelineServer
from paddle_serving_server.pipeline.proto import pipeline_service_pb2
from paddle_serving_server.pipeline.channel import ChannelDataErrcode

_LOGGER = logging.getLogger()
user_handler = logging.StreamHandler()
user_handler.setLevel(logging.INFO)
user_handler.setFormatter(
    logging.Formatter(
        "%(levelname)s %(asctime)s [%(filename)s:%(lineno)d] %(message)s"))
_LOGGER.addHandler(user_handler)


class ImdbRequestOp(RequestOp):
    def init_op(self):
        self.imdb_dataset = IMDBDataset()
        self.imdb_dataset.load_resource('imdb.vocab')

    def unpack_request_package(self, request):
        dictdata = {}
        for idx, key in enumerate(request.key):
            if key != "words":
                continue
            words = request.value[idx]
            word_ids, _ = self.imdb_dataset.get_words_and_label(words)
            word_len = len(word_ids)
            dictdata[key] = np.array(word_ids).reshape(word_len, 1)
            dictdata["{}.lod".format(key)] = np.array([0, word_len])

        log_id = None
        if request.logid is not None:
            log_id = request.logid
        return dictdata, log_id, None, ""


class CombineOp(Op):
    def preprocess(self, input_data, data_id, log_id):
        #_LOGGER.info("Enter CombineOp::preprocess")
        combined_prediction = 0
        for op_name, data in input_data.items():
            _LOGGER.info("{}: {}".format(op_name, data["prediction"]))
            combined_prediction += data["prediction"]
        data = {"prediction": combined_prediction / 2}
        return data, False, None, ""


class ImdbResponseOp(ResponseOp):
    # Here ImdbResponseOp is consistent with the default ResponseOp implementation
    def pack_response_package(self, channeldata):
        resp = pipeline_service_pb2.Response()
        resp.err_no = channeldata.error_code
        if resp.err_no == ChannelDataErrcode.OK.value:
            feed = channeldata.parse()
            # ndarray to string
            for name, var in feed.items():
                resp.value.append(var.__repr__())
                resp.key.append(name)
        else:
            resp.err_msg = channeldata.error_info
        return resp


read_op = ImdbRequestOp()


class BowOp(Op):
    def init_op(self):
        pass


class CnnOp(Op):
    def init_op(self):
        pass


bow_op = BowOp("bow", input_ops=[read_op])
cnn_op = CnnOp("cnn", input_ops=[read_op])
combine_op = CombineOp("combine", input_ops=[bow_op, cnn_op])

# fetch output of bow_op
#response_op = ImdbResponseOp(input_ops=[bow_op])

# fetch output of combine_op
response_op = ImdbResponseOp(input_ops=[combine_op])

# use default ResponseOp implementation
#response_op = ResponseOp(input_ops=[combine_op])

server = PipelineServer()
server.set_response_op(response_op)
server.prepare_server('config.yml')
server.run_server()
