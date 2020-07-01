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

from paddle_serving_client import Client
from paddle_serving_app.reader import OCRReader
import cv2
import sys
import numpy as np
import os
from paddle_serving_client import Client
from paddle_serving_app.reader import Sequential, File2Image, ResizeByFactor
from paddle_serving_app.reader import Div, Normalize, Transpose
from paddle_serving_app.reader import DBPostProcess, FilterBoxes
import time
import re


def sorted_boxes(dt_boxes):
    """
    Sort text boxes in order from top to bottom, left to right
    args:
        dt_boxes(array):detected text boxes with shape [4, 2]
    return:
        sorted boxes(array) with shape [4, 2]
    """
    num_boxes = dt_boxes.shape[0]
    sorted_boxes = sorted(dt_boxes, key=lambda x: (x[0][1], x[0][0]))
    _boxes = list(sorted_boxes)

    for i in range(num_boxes - 1):
        if abs(_boxes[i+1][0][1] - _boxes[i][0][1]) < 10 and \
            (_boxes[i + 1][0][0] < _boxes[i][0][0]):
            tmp = _boxes[i]
            _boxes[i] = _boxes[i + 1]
            _boxes[i + 1] = tmp
    return _boxes


def get_rotate_crop_image(img, points):
    #img = cv2.imread(img)
    img_height, img_width = img.shape[0:2]
    left = int(np.min(points[:, 0]))
    right = int(np.max(points[:, 0]))
    top = int(np.min(points[:, 1]))
    bottom = int(np.max(points[:, 1]))
    img_crop = img[top:bottom, left:right, :].copy()
    points[:, 0] = points[:, 0] - left
    points[:, 1] = points[:, 1] - top
    img_crop_width = int(np.linalg.norm(points[0] - points[1]))
    img_crop_height = int(np.linalg.norm(points[0] - points[3]))
    pts_std = np.float32([[0, 0], [img_crop_width, 0], \
                          [img_crop_width, img_crop_height], [0, img_crop_height]])
    M = cv2.getPerspectiveTransform(points, pts_std)
    dst_img = cv2.warpPerspective(
        img_crop,
        M, (img_crop_width, img_crop_height),
        borderMode=cv2.BORDER_REPLICATE)
    dst_img_height, dst_img_width = dst_img.shape[0:2]
    if dst_img_height * 1.0 / dst_img_width >= 1.5:
        dst_img = np.rot90(dst_img)
    return dst_img


def read_det_box_file(filename):
    with open(filename, 'r') as f:
        line = f.readline()
        a, b, c = int(line.split(' ')[0]), int(line.split(' ')[1]), int(
            line.split(' ')[2])
        dt_boxes = np.zeros((a, b, c)).astype(np.float32)
        line = f.readline()
        for i in range(a):
            for j in range(b):
                line = f.readline()
                dt_boxes[i, j, 0], dt_boxes[i, j, 1] = float(
                    line.split(' ')[0]), float(line.split(' ')[1])
            line = f.readline()


def resize_norm_img(img, max_wh_ratio):
    import math
    imgC, imgH, imgW = 3, 32, 320
    imgW = int(32 * max_wh_ratio)
    h = img.shape[0]
    w = img.shape[1]
    ratio = w / float(h)
    if math.ceil(imgH * ratio) > imgW:
        resized_w = imgW
    else:
        resized_w = int(math.ceil(imgH * ratio))
    resized_image = cv2.resize(img, (resized_w, imgH))
    resized_image = resized_image.astype('float32')
    resized_image = resized_image.transpose((2, 0, 1)) / 255
    resized_image -= 0.5
    resized_image /= 0.5
    padding_im = np.zeros((imgC, imgH, imgW), dtype=np.float32)
    padding_im[:, :, 0:resized_w] = resized_image
    return padding_im


def main():
    client1 = Client()
    client1.load_client_config("ocr_det_client/serving_client_conf.prototxt")
    client1.connect(["127.0.0.1:9293"])

    client2 = Client()
    client2.load_client_config("ocr_rec_client/serving_client_conf.prototxt")
    client2.connect(["127.0.0.1:9292"])

    read_image_file = File2Image()
    preprocess = Sequential([
        ResizeByFactor(32, 960), Div(255),
        Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225]), Transpose(
            (2, 0, 1))
    ])
    post_func = DBPostProcess({
        "thresh": 0.3,
        "box_thresh": 0.5,
        "max_candidates": 1000,
        "unclip_ratio": 1.5,
        "min_size": 3
    })

    filter_func = FilterBoxes(10, 10)
    ocr_reader = OCRReader()
    files = [
        "./imgs/{}".format(f) for f in os.listdir('./imgs')
        if re.match(r'[0-9]+.*\.jpg|[0-9]+.*\.png', f)
    ]
    #files = ["2.jpg"]*30
    #files = ["rctw/rctw/train/images/image_{}.jpg".format(i) for i in range(500)]
    time_all = 0
    time_det_all = 0
    time_rec_all = 0
    for name in files:
        #print(name)
        im = read_image_file(name)
        ori_h, ori_w, _ = im.shape
        time1 = time.time()
        img = preprocess(im)
        _, new_h, new_w = img.shape
        ratio_list = [float(new_h) / ori_h, float(new_w) / ori_w]
        #print(new_h, new_w, ori_h, ori_w)
        time_before_det = time.time()
        outputs = client1.predict(feed={"image": img}, fetch=["concat_1.tmp_0"])
        time_after_det = time.time()
        time_det_all += (time_after_det - time_before_det)
        #print(outputs)
        dt_boxes_list = post_func(outputs["concat_1.tmp_0"], [ratio_list])
        dt_boxes = filter_func(dt_boxes_list[0], [ori_h, ori_w])
        dt_boxes = sorted_boxes(dt_boxes)
        feed_list = []
        img_list = []
        max_wh_ratio = 0
        for i, dtbox in enumerate(dt_boxes):
            boximg = get_rotate_crop_image(im, dt_boxes[i])
            img_list.append(boximg)
            h, w = boximg.shape[0:2]
            wh_ratio = w * 1.0 / h
            max_wh_ratio = max(max_wh_ratio, wh_ratio)
        for img in img_list:
            norm_img = resize_norm_img(img, max_wh_ratio)
            #norm_img = norm_img[np.newaxis, :]
            feed = {"image": norm_img}
            feed_list.append(feed)
        #fetch = ["ctc_greedy_decoder_0.tmp_0", "softmax_0.tmp_0"]
        fetch = ["ctc_greedy_decoder_0.tmp_0"]
        time_before_rec = time.time()
        if len(feed_list) == 0:
            continue
        fetch_map = client2.predict(feed=feed_list, fetch=fetch)
        time_after_rec = time.time()
        time_rec_all += (time_after_rec - time_before_rec)
        rec_res = ocr_reader.postprocess(fetch_map)
        #for res in rec_res:
        #    print(res[0].encode("utf-8"))
        time2 = time.time()
        time_all += (time2 - time1)
    print("rpc+det time: {}".format(time_all / len(files)))


if __name__ == '__main__':
    main()
