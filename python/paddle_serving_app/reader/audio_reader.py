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
audio feature op
"""

import ffmpeg
import numpy
import logging
import cv2
import adpredictor as adpredictor

try:
    from paddle_serving_server.pipeline import Op
except Exception as e:
    from paddle_serving_server_gpu.pipeline import Op

_LOGGER = logging.getLogger()


class AudioFeatureOp(Op):
    """ 
    audio feature op class
    """

    def extract_audio_from_video(self, local_video_path):
        """
        extract audios from video
        Params:
            local_video_path, string
        Returns:
            audio pcm s16 le raw data for success, None for failed
        """
        out, _ = (ffmpeg.input(local_video_path, ).output(
            '-', format='s16le', acodec='pcm_s16le', ac=1, ar='16k')
                  .overwrite_output().run(capture_stdout=True))
        result = numpy.frombuffer(out, dtype=numpy.short)
        return result