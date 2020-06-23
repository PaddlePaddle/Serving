#   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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


class PipelineService(pipeline_service_pb2_grpc.PipelineServiceServicer):
    def __init__(self, in_channel, out_channel, retry=2):
        super(PipelineService, self).__init__()
        pass


class PipelineServer(object):
    def __init__(self):
        pass

    def set_response_op(self, response_op):
        pass

    def prepare_server(self, yml_file):
        pass

    def run_server(self):
        pass
