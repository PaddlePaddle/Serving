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

import cv2
import copy
import numpy as np
import math
import re
import sys
import argparse
from paddle_serving_app.reader import Sequential, Resize, Transpose, Div, Normalize


class CharacterOps(object):
    """ Convert between text-label and text-index """

    def __init__(self, config):
        self.character_type = config['character_type']
        self.loss_type = config['loss_type']
        if self.character_type == "en":
            self.character_str = "0123456789abcdefghijklmnopqrstuvwxyz"
            dict_character = list(self.character_str)
        elif self.character_type == "ch":
            character_dict_path = config['character_dict_path']
            self.character_str = ""
            with open(character_dict_path, "rb") as fin:
                lines = fin.readlines()
                for line in lines:
                    line = line.decode('utf-8').strip("\n").strip("\r\n")
                    self.character_str += line
            dict_character = list(self.character_str)
        elif self.character_type == "en_sensitive":
            # same with ASTER setting (use 94 char).
            self.character_str = string.printable[:-6]
            dict_character = list(self.character_str)
        else:
            self.character_str = None
        assert self.character_str is not None, \
            "Nonsupport type of the character: {}".format(self.character_str)
        self.beg_str = "sos"
        self.end_str = "eos"
        if self.loss_type == "attention":
            dict_character = [self.beg_str, self.end_str] + dict_character
        self.dict = {}
        for i, char in enumerate(dict_character):
            self.dict[char] = i
        self.character = dict_character

    def encode(self, text):
        """convert text-label into text-index.
        input:
            text: text labels of each image. [batch_size]

        output:
            text: concatenated text index for CTCLoss.
                    [sum(text_lengths)] = [text_index_0 + text_index_1 + ... + text_index_(n - 1)]
            length: length of each text. [batch_size]
        """
        if self.character_type == "en":
            text = text.lower()

        text_list = []
        for char in text:
            if char not in self.dict:
                continue
            text_list.append(self.dict[char])
        text = np.array(text_list)
        return text

    def decode(self, text_index, is_remove_duplicate=False):
        """ convert text-index into text-label. """
        char_list = []
        char_num = self.get_char_num()

        if self.loss_type == "attention":
            beg_idx = self.get_beg_end_flag_idx("beg")
            end_idx = self.get_beg_end_flag_idx("end")
            ignored_tokens = [beg_idx, end_idx]
        else:
            ignored_tokens = [char_num]

        for idx in range(len(text_index)):
            if text_index[idx] in ignored_tokens:
                continue
            if is_remove_duplicate:
                if idx > 0 and text_index[idx - 1] == text_index[idx]:
                    continue
            char_list.append(self.character[text_index[idx]])
        text = ''.join(char_list)
        return text

    def get_char_num(self):
        return len(self.character)

    def get_beg_end_flag_idx(self, beg_or_end):
        if self.loss_type == "attention":
            if beg_or_end == "beg":
                idx = np.array(self.dict[self.beg_str])
            elif beg_or_end == "end":
                idx = np.array(self.dict[self.end_str])
            else:
                assert False, "Unsupport type %s in get_beg_end_flag_idx"\
                    % beg_or_end
            return idx
        else:
            err = "error in get_beg_end_flag_idx when using the loss %s"\
                % (self.loss_type)
            assert False, err


class OCRReader(object):
    def __init__(self,
                 algorithm="CRNN",
                 image_shape=[3, 32, 320],
                 char_type="ch",
                 batch_num=1,
                 char_dict_path="./ppocr_keys_v1.txt"):
        self.rec_image_shape = image_shape
        self.character_type = char_type
        self.rec_batch_num = batch_num
        char_ops_params = {}
        char_ops_params["character_type"] = char_type
        char_ops_params["character_dict_path"] = char_dict_path
        char_ops_params['loss_type'] = 'ctc'
        self.char_ops = CharacterOps(char_ops_params)

    def resize_norm_img(self, img, max_wh_ratio):
        imgC, imgH, imgW = self.rec_image_shape
        if self.character_type == "ch":
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

    def preprocess(self, img_list):
        img_num = len(img_list)
        norm_img_batch = []
        max_wh_ratio = 0
        for ino in range(img_num):
            h, w = img_list[ino].shape[0:2]
            wh_ratio = w * 1.0 / h
            max_wh_ratio = max(max_wh_ratio, wh_ratio)
        for ino in range(img_num):
            norm_img = self.resize_norm_img(img_list[ino], max_wh_ratio)
            norm_img = norm_img[np.newaxis, :]
            norm_img_batch.append(norm_img)
        norm_img_batch = np.concatenate(norm_img_batch)
        norm_img_batch = norm_img_batch.copy()

        return norm_img_batch[0]

    def postprocess(self, outputs, with_score=False):
        rec_res = []
        rec_idx_lod = outputs["ctc_greedy_decoder_0.tmp_0.lod"]
        rec_idx_batch = outputs["ctc_greedy_decoder_0.tmp_0"]
        if with_score:
            predict_lod = outputs["softmax_0.tmp_0.lod"]
        for rno in range(len(rec_idx_lod) - 1):
            beg = rec_idx_lod[rno]
            end = rec_idx_lod[rno + 1]
            if isinstance(rec_idx_batch, list):
                rec_idx_tmp = [x[0] for x in rec_idx_batch[beg:end]]
            else:  #nd array
                rec_idx_tmp = rec_idx_batch[beg:end, 0]
            preds_text = self.char_ops.decode(rec_idx_tmp)
            if with_score:
                beg = predict_lod[rno]
                end = predict_lod[rno + 1]
                if isinstance(outputs["softmax_0.tmp_0"], list):
                    outputs["softmax_0.tmp_0"] = np.array(outputs[
                        "softmax_0.tmp_0"]).astype(np.float32)
                probs = outputs["softmax_0.tmp_0"][beg:end, :]
                ind = np.argmax(probs, axis=1)
                blank = probs.shape[1]
                valid_ind = np.where(ind != (blank - 1))[0]
                score = np.mean(probs[valid_ind, ind[valid_ind]])
                rec_res.append([preds_text, score])
            else:
                rec_res.append([preds_text])
        return rec_res
