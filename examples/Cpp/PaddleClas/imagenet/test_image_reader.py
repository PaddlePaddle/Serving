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

from paddle_serving_app.reader.image_reader import String2Image, Base64ToImage, Sequential
import base64


def test_String2Image():
    with open("./daisy.jpg") as f:
        img_str = f.read()
    seq = Sequential([String2Image()])
    img = seq(img_str)
    assert (img.shape == (563, 500, 3))


def test_Base64ToImage():
    with open("./daisy.jpg") as f:
        img_str = f.read()
    seq = Sequential([Base64ToImage()])
    img = seq(base64.b64encode(img_str))
    assert (img.shape == (563, 500, 3))


if __name__ == "__main__":
    test_String2Image()
    test_Base64ToImage()
