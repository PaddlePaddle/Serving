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

"""
hwextract lib use case
"""
import paddle_serving_app.reader.hwextract
import sys
if __name__ == '__main__':
    handler = hwextract.HwExtractFrameJpeg(0)
    # 0, gpu card index
    # if you want BGRA Raw Data, plz use HwExtractBGRARaw
    handler.init_handler()
    # init once can decode many videos
    video_file_name = sys.argv[1]
    # for now just support h264 codec
    frame_list = []
    try:
        frame_list = handler.extract_frame(video_file_name, 1)
        # specifiy file name and fps you want to extract, 0 for all frame
    except Exception as e_frame:
        print("Failed to cutframe, exception[%s]" % (e_frame))
        sys.exit(1)
    for item in frame_list:
        print "i am a item in frame_list"
        # do something, for instance
        # jpeg_array = np.array(item, copy=False)
        # img = cv2.imdecode(item, cv2.IMREAD_COLOR)
        # etc.....
        item.free_memory()
        # have to release memory