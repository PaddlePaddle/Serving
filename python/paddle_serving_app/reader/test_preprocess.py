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

import unittest
import sys
import numpy as np
import cv2
from paddle_serving_app.reader import Sequential, Resize, File2Image
import libgpupreprocess as pp
import libhwextract


class TestOperators(unittest.TestCase):
    """
    test all operators, e.g. Div, Normalize
    """

    def test_div(self):
        height = 4
        width = 5
        channels = 3
        value = 255.0
        img = np.arange(height * width * channels).reshape(
            [height, width, channels])
        seq = Sequential(
            [pp.Image2Gpubuffer(), pp.Div(value), pp.Gpubuffer2Image()])
        result = seq(img).reshape(-1)
        for i in range(0, result.size):
            self.assertAlmostEqual(i / value, result[i], 5)

    def test_sub(self):
        height = 4
        width = 5
        channels = 3
        img = np.arange(height * width * channels).reshape(
            [height, width, channels])

        # input size is an int
        value = 10.0
        seq = Sequential(
            [pp.Image2Gpubuffer(), pp.Sub(value), pp.Gpubuffer2Image()])
        result = seq(img).reshape(-1)
        for i in range(0, result.size):
            self.assertEqual(i - value, result[i])

        # input size is a sequence
        values = (9, 4, 2)
        seq = Sequential(
            [pp.Image2Gpubuffer(), pp.Sub(values), pp.Gpubuffer2Image()])
        result = seq(img)
        for i in range(0, result.shape[0]):
            for j in range(0, result.shape[1]):
                for k in range(0, result.shape[2]):
                    self.assertEqual(result[i][j][k], img[i][j][k] - values[k])

    def test_normalize(self):
        height = 4
        width = 5
        channels = 3
        img = np.random.rand(height, width, channels)
        mean = [5.0, 5.0, 5.0]
        std = [2.0, 2.0, 2.0]
        seq = Sequential([
            pp.Image2Gpubuffer(), pp.Normalize(mean, std), pp.Gpubuffer2Image()
        ])
        result = seq(img)
        for i in range(0, height):
            for j in range(0, width):
                for k in range(0, channels):
                    self.assertAlmostEqual((img[i][j][k] - mean[k]) / std[k],
                                           result[i][j][k], 5)

    def test_center_crop(self):
        height = 9
        width = 7
        channels = 3
        img = np.arange(height * width * channels).reshape(
            [height, width, channels])
        new_size = 5
        seq = Sequential([
            pp.Image2Gpubuffer(), pp.CenterCrop(new_size), pp.Gpubuffer2Image()
        ])
        result = seq(img)
        self.assertEqual(result.shape[0], new_size)
        self.assertEqual(result.shape[1], new_size)
        self.assertEqual(result.shape[2], channels)

    def test_resize(self):
        height = 9
        width = 5
        channels = 3
        img = np.arange(height * width).reshape([height, width, 1]) * np.ones(
            (1, channels))

        # input size is an int
        for new_size in [3, 10]:
            seq_gpu = Sequential([
                pp.Image2Gpubuffer(), pp.Resize(new_size), pp.Gpubuffer2Image()
            ])
            seq_paddle = Sequential([Resize(new_size)])
            result_gpu = seq_gpu(img)
            result_paddle = seq_paddle(img)
            self.assertEqual(result_gpu.shape, result_paddle.shape)
            for i in range(0, result_gpu.shape[0]):
                for j in range(0, result_gpu.shape[1]):
                    for k in range(0, result_gpu.shape[2]):
                        self.assertAlmostEqual(result_gpu[i][j][k],
                                               result_paddle[i][j][k], 5)

        # input size is a sequence
        for new_height, new_width in [(7, 3), (15, 10)]:
            seq_gpu = Sequential([
                pp.Image2Gpubuffer(), pp.Resize((new_width, new_height)),
                pp.Gpubuffer2Image()
            ])
            seq_paddle = Sequential([Resize((new_width, new_height))])
            result_gpu = seq_gpu(img)
            result_paddle = seq_paddle(img)
            self.assertEqual(result_gpu.shape, result_paddle.shape)
            for i in range(0, result_gpu.shape[0]):
                for j in range(0, result_gpu.shape[1]):
                    for k in range(0, result_gpu.shape[2]):
                        self.assertAlmostEqual(result_gpu[i][j][k],
                                               result_paddle[i][j][k], 5)

    def test_resize_fixed_point(self):
        new_height = 256
        new_width = 256 * 4 / 3
        seq = Sequential([
            File2Image(), pp.Image2Gpubuffer(), pp.Resize(
                (new_width, new_height), use_fixed_point=True),
            pp.Gpubuffer2Image()
        ])
        img = seq("./capture_16.bmp")
        img = np.resize(img, (new_height, new_width * 3))
        img_vis = np.loadtxt("./cap_resize_16.raw")
        img_resize_diff = img_vis - img
        self.assertEqual(np.all(img_resize_diff == 0), True)


if __name__ == '__main__':
    unittest.main()
