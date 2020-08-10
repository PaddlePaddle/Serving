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
"""
frame extract op
"""

import logging
import sys
import frame_extractor as frame_ext
try:
    from paddle_serving_server.pipeline import Op
except Exception as e:
    from paddle_serving_server_gpu.pipeline import Op

_LOGGER = logging.getLogger(__name__)


class FrameExtractOp(Op):
    """ frame extract op """

    def init_op(self):
        """
        overwrite init_op function to load custom resources.

        note: 
        since only one frame_extractor instance can run on
        each gpu card, this op can only start one single thread.
        """
        pipeline_config = util.load_config(sys.argv[1])
        self.is_thread = pipeline_config["dag"]["is_thread_op"]
        op_config = util.load_config(pipeline_config["op_config"])

        self.frame_ext = frame_ext.FrameExt()
        self.frame_ext.init_handler(op_config["frame_extract_card"])

    def preprocess(self, input_dict):
        """ extract frame """
        (_, input_data), = input_dict.items()
        video_data = input_data.get("video_data")
        is_pcm_raw_data = input_data.get("is_pcm_raw_data")
        seq = input_data.get("seq", None)
        if is_pcm_raw_data:
            raise Exception("not support pcm_raw_data")

        frame_list = self.frame_ext.extract_frame(video_data)
        frame_batches = self.frame_ext.chunks(frame_list, 8)

        if not self.is_thread:
            # TODO: object hwextract.HwFrameResult could
            # not be serialized correctly.
            frame_batches = list(frame_batches)
            #for frame_list in frame_batches:
            #    width = frame_list[0].width()
            #    height = frame_list[0].height()
            #    _LOGGER.error("[FrameExt]({}) width: {}, height: {}"
            #            .format(type(frame_list[0]), width, height))
            #    break
        output_dict = {"frame_batches": frame_batches, "seq": None}
        return output_dict
