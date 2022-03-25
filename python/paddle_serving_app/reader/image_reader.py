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
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
import cv2
import os
import numpy as np
import base64
import sys
from . import functional as F
from PIL import Image, ImageDraw
from shapely.geometry import Polygon
import pyclipper
import json

_cv2_interpolation_to_str = {cv2.INTER_LINEAR: "cv2.INTER_LINEAR", None: "None"}
py_version = sys.version_info[0]
if py_version == 2:
    import urllib
else:
    import urllib.request as urllib


def generate_colormap(num_classes):
    color_map = num_classes * [0, 0, 0]
    for i in range(0, num_classes):
        j = 0
        lab = i
        while lab:
            color_map[i * 3] |= (((lab >> 0) & 1) << (7 - j))
            color_map[i * 3 + 1] |= (((lab >> 1) & 1) << (7 - j))
            color_map[i * 3 + 2] |= (((lab >> 2) & 1) << (7 - j))
            j += 1
            lab >>= 3
    color_map = [color_map[i:i + 3] for i in range(0, len(color_map), 3)]
    return color_map


class DBPostProcess(object):
    """
    The post process for Differentiable Binarization (DB).
    """

    def __init__(self, params):
        self.thresh = params['thresh']
        self.box_thresh = params['box_thresh']
        self.max_candidates = params['max_candidates']
        self.unclip_ratio = params['unclip_ratio']
        self.min_size = 3

    def boxes_from_bitmap(self, pred, _bitmap, dest_width, dest_height):
        '''
        _bitmap: single map with shape (1, H, W),
                whose values are binarized as {0, 1}
        '''

        bitmap = _bitmap
        height, width = bitmap.shape

        outs = cv2.findContours((bitmap * 255).astype(np.uint8), cv2.RETR_LIST,
                                cv2.CHAIN_APPROX_SIMPLE)
        if len(outs) == 3:
            img, contours, _ = outs[0], outs[1], outs[2]
        elif len(outs) == 2:
            contours, _ = outs[0], outs[1]

        num_contours = min(len(contours), self.max_candidates)
        boxes = np.zeros((num_contours, 4, 2), dtype=np.int16)
        scores = np.zeros((num_contours, ), dtype=np.float32)

        for index in range(num_contours):
            contour = contours[index]
            points, sside = self.get_mini_boxes(contour)
            if sside < self.min_size:
                continue
            points = np.array(points)
            score = self.box_score_fast(pred, points.reshape(-1, 2))
            if self.box_thresh > score:
                continue

            box = self.unclip(points).reshape(-1, 1, 2)
            box, sside = self.get_mini_boxes(box)
            if sside < self.min_size + 2:
                continue
            box = np.array(box)
            if not isinstance(dest_width, int):
                dest_width = dest_width.item()
                dest_height = dest_height.item()

            box[:, 0] = np.clip(
                np.round(box[:, 0] / width * dest_width), 0, dest_width)
            box[:, 1] = np.clip(
                np.round(box[:, 1] / height * dest_height), 0, dest_height)
            boxes[index, :, :] = box.astype(np.int16)
            scores[index] = score
        return boxes, scores

    def unclip(self, box):
        unclip_ratio = self.unclip_ratio
        poly = Polygon(box)
        distance = poly.area * unclip_ratio / poly.length
        offset = pyclipper.PyclipperOffset()
        offset.AddPath(box, pyclipper.JT_ROUND, pyclipper.ET_CLOSEDPOLYGON)
        expanded = np.array(offset.Execute(distance))
        return expanded

    def get_mini_boxes(self, contour):
        bounding_box = cv2.minAreaRect(contour)
        points = sorted(list(cv2.boxPoints(bounding_box)), key=lambda x: x[0])

        index_1, index_2, index_3, index_4 = 0, 1, 2, 3
        if points[1][1] > points[0][1]:
            index_1 = 0
            index_4 = 1
        else:
            index_1 = 1
            index_4 = 0
        if points[3][1] > points[2][1]:
            index_2 = 2
            index_3 = 3
        else:
            index_2 = 3
            index_3 = 2

        box = [
            points[index_1], points[index_2], points[index_3], points[index_4]
        ]
        return box, min(bounding_box[1])

    def box_score_fast(self, bitmap, _box):
        h, w = bitmap.shape[:2]
        box = _box.copy()
        xmin = np.clip(np.floor(box[:, 0].min()).astype(np.int32), 0, w - 1)
        xmax = np.clip(np.ceil(box[:, 0].max()).astype(np.int32), 0, w - 1)
        ymin = np.clip(np.floor(box[:, 1].min()).astype(np.int32), 0, h - 1)
        ymax = np.clip(np.ceil(box[:, 1].max()).astype(np.int32), 0, h - 1)

        mask = np.zeros((ymax - ymin + 1, xmax - xmin + 1), dtype=np.uint8)
        box[:, 0] = box[:, 0] - xmin
        box[:, 1] = box[:, 1] - ymin
        cv2.fillPoly(mask, box.reshape(1, -1, 2).astype(np.int32), 1)
        return cv2.mean(bitmap[ymin:ymax + 1, xmin:xmax + 1], mask)[0]

    def __call__(self, pred, ratio_list):
        pred = pred[:, 0, :, :]
        segmentation = pred > self.thresh

        boxes_batch = []
        for batch_index in range(pred.shape[0]):
            height, width = pred.shape[-2:]
            tmp_boxes, tmp_scores = self.boxes_from_bitmap(
                pred[batch_index], segmentation[batch_index], width, height)

            boxes = []
            for k in range(len(tmp_boxes)):
                if tmp_scores[k] > self.box_thresh:
                    boxes.append(tmp_boxes[k])
            if len(boxes) > 0:
                boxes = np.array(boxes)

                ratio_h, ratio_w = ratio_list[batch_index]
                boxes[:, :, 0] = boxes[:, :, 0] / ratio_w
                boxes[:, :, 1] = boxes[:, :, 1] / ratio_h

            boxes_batch.append(boxes)
        return boxes_batch

    def __repr__(self):
        return self.__class__.__name__ + \
            " thresh: {1}, box_thresh: {2}, max_candidates: {3}, unclip_ratio: {4}, min_size: {5}".format(
                self.thresh, self.box_thresh, self.max_candidates, self.unclip_ratio, self.min_size)


class FilterBoxes(object):
    def __init__(self, width, height):
        self.filter_width = width
        self.filter_height = height

    def order_points_clockwise(self, pts):
        """
        reference from: https://github.com/jrosebr1/imutils/blob/master/imutils/perspective.py
        # sort the points based on their x-coordinates
        """
        xSorted = pts[np.argsort(pts[:, 0]), :]

        # grab the left-most and right-most points from the sorted
        # x-roodinate points
        leftMost = xSorted[:2, :]
        rightMost = xSorted[2:, :]

        # now, sort the left-most coordinates according to their
        # y-coordinates so we can grab the top-left and bottom-left
        # points, respectively
        leftMost = leftMost[np.argsort(leftMost[:, 1]), :]
        (tl, bl) = leftMost

        rightMost = rightMost[np.argsort(rightMost[:, 1]), :]
        (tr, br) = rightMost

        rect = np.array([tl, tr, br, bl], dtype="float32")
        return rect

    def clip_det_res(self, points, img_height, img_width):
        for pno in range(4):
            points[pno, 0] = int(min(max(points[pno, 0], 0), img_width - 1))
            points[pno, 1] = int(min(max(points[pno, 1], 0), img_height - 1))
        return points

    def __call__(self, dt_boxes, image_shape):
        img_height, img_width = image_shape[0:2]
        dt_boxes_new = []
        for box in dt_boxes:
            box = self.order_points_clockwise(box)
            box = self.clip_det_res(box, img_height, img_width)
            rect_width = int(np.linalg.norm(box[0] - box[1]))
            rect_height = int(np.linalg.norm(box[0] - box[3]))
            if rect_width <= self.filter_width or \
               rect_height <= self.filter_height:
                continue
            dt_boxes_new.append(box)
        dt_boxes = np.array(dt_boxes_new)
        return dt_boxes

    def __repr__(self):
        return self.__class__.__name__ + " filter_width: {1}, filter_height: {2}".format(
            self.filter_width, self.filter_height)


class SegPostprocess(object):
    def __init__(self, class_num):
        self.class_num = class_num

    def __call__(self, image_with_result):
        if "filename" not in image_with_result:
            raise ("filename should be specified in postprocess")
        img_name = image_with_result["filename"]
        ori_img = cv2.imread(img_name, -1)
        ori_shape = ori_img.shape
        mask = None
        for key in image_with_result:
            if ".lod" in key or "filename" in key:
                continue
            mask = image_with_result[key]
        if mask is None:
            raise ("segment mask should be specified in postprocess")
        mask = mask.astype("uint8")
        mask_png = mask.reshape((512, 512, 1))
        #score_png = mask_png[:, :, np.newaxis]
        score_png = mask_png
        score_png = np.concatenate([score_png] * 3, axis=2)
        color_map = generate_colormap(self.class_num)
        for i in range(score_png.shape[0]):
            for j in range(score_png.shape[1]):
                score_png[i, j] = color_map[score_png[i, j, 0]]
        ext_pos = img_name.rfind(".")
        img_name_fix = img_name[:ext_pos] + "_" + img_name[ext_pos + 1:]
        mask_save_name = img_name_fix + "_mask.png"
        cv2.imwrite(mask_save_name, mask_png, [cv2.CV_8UC1])
        vis_result_name = img_name_fix + "_result.png"
        result_png = score_png

        result_png = cv2.resize(
            result_png, (ori_shape[1], ori_shape[0]),
            fx=0,
            fy=0,
            interpolation=cv2.INTER_CUBIC)
        cv2.imwrite(vis_result_name, result_png, [cv2.CV_8UC1])


class RCNNPostprocess(object):
    def __init__(self, label_file, output_dir, resize_shape=None):
        self.output_dir = output_dir
        self.label_file = label_file
        self.label_list = []
        self.resize_shape = resize_shape
        with open(label_file) as fin:
            for line in fin:
                self.label_list.append(line.strip())
        self.clsid2catid = {i: i for i in range(len(self.label_list))}
        self.catid2name = {i: name for i, name in enumerate(self.label_list)}

    def _offset_to_lengths(self, lod):
        offset = lod[0]
        lengths = [offset[i + 1] - offset[i] for i in range(len(offset) - 1)]
        return [lengths]

    def _bbox2out(self, results, clsid2catid, is_bbox_normalized=False):
        xywh_res = []
        for t in results:
            bboxes = t['bbox'][0]
            lengths = t['bbox'][1][0]
            if bboxes.shape == (1, 1) or bboxes is None:
                continue

            k = 0
            for i in range(len(lengths)):
                num = lengths[i]
                for j in range(num):
                    dt = bboxes[k]
                    clsid, score, xmin, ymin, xmax, ymax = dt.tolist()
                    catid = (clsid2catid[int(clsid)])

                    if is_bbox_normalized:
                        xmin, ymin, xmax, ymax = \
                            self.clip_bbox([xmin, ymin, xmax, ymax])
                        w = xmax - xmin
                        h = ymax - ymin
                        im_shape = t['im_shape'].tolist()
                        im_height, im_width = int(im_shape[0]), int(im_shape[1])
                        xmin *= im_width
                        ymin *= im_height
                        w *= im_width
                        h *= im_height
                    else:
                        w = xmax - xmin + 1
                        h = ymax - ymin + 1

                    bbox = [xmin, ymin, w, h]
                    coco_res = {
                        'category_id': catid,
                        'bbox': bbox,
                        'score': score
                    }
                    xywh_res.append(coco_res)
                    k += 1
        return xywh_res

    def _get_bbox_result(self, fetch_map, fetch_name, clsid2catid):
        result = {}
        is_bbox_normalized = False
        output = fetch_map[fetch_name]
        lod = [fetch_map[fetch_name + '.lod']]
        lengths = self._offset_to_lengths(lod)
        np_data = np.array(output)
        result['bbox'] = (np_data, lengths)
        result['im_id'] = np.array([[0]])

        bbox_results = self._bbox2out([result], clsid2catid, is_bbox_normalized)
        return bbox_results

    def color_map(self, num_classes):
        color_map = num_classes * [0, 0, 0]
        for i in range(0, num_classes):
            j = 0
            lab = i
            while lab:
                color_map[i * 3] |= (((lab >> 0) & 1) << (7 - j))
                color_map[i * 3 + 1] |= (((lab >> 1) & 1) << (7 - j))
                color_map[i * 3 + 2] |= (((lab >> 2) & 1) << (7 - j))
                j += 1
                lab >>= 3
        color_map = np.array(color_map).reshape(-1, 3)
        return color_map

    def draw_bbox(self, image, catid2name, bboxes, threshold, color_list):
        """
        draw bbox on image
        """
        draw = ImageDraw.Draw(image)

        for dt in np.array(bboxes):
            catid, bbox, score = dt['category_id'], dt['bbox'], dt['score']
            if score < threshold:
                continue

            xmin, ymin, w, h = bbox
            xmax = xmin + w
            ymax = ymin + h

            img_w, img_h = image.size
            if self.resize_shape is not None:
                xmin = xmin * img_w / self.resize_shape[0]
                xmax = xmax * img_w / self.resize_shape[0]
                ymin = ymin * img_h / self.resize_shape[1]
                ymax = ymax * img_h / self.resize_shape[1]

            color = tuple(color_list[catid])

            # draw bbox
            draw.line(
                [(xmin, ymin), (xmin, ymax), (xmax, ymax), (xmax, ymin),
                 (xmin, ymin)],
                width=2,
                fill=color)

            # draw label
            text = "{} {:.2f}".format(catid2name[catid], score)
            tw, th = draw.textsize(text)
            draw.rectangle(
                [(xmin + 1, ymin - th), (xmin + tw + 1, ymin)], fill=color)
            draw.text((xmin + 1, ymin - th), text, fill=(255, 255, 255))

        return image

    def visualize(self, infer_img, bbox_results, catid2name, num_classes):
        image = Image.open(infer_img).convert('RGB')
        color_list = self.color_map(num_classes)
        image = self.draw_bbox(image, self.catid2name, bbox_results, 0.5,
                               color_list)
        image_path = os.path.split(infer_img)[-1]
        if not os.path.exists(self.output_dir):
            os.makedirs(self.output_dir)
        out_path = os.path.join(self.output_dir, image_path)
        image.save(out_path, quality=95)

    def __call__(self, image_with_bbox, visualize=True):
        fetch_name = ""
        for key in image_with_bbox:
            if key == "image":
                continue
            if ".lod" in key or "im_shape" in key:
                continue
            fetch_name = key
        bbox_result = self._get_bbox_result(image_with_bbox, fetch_name,
                                            self.clsid2catid)
        if os.path.isdir(self.output_dir) is False:
            os.mkdir(self.output_dir)
        if visualize is False:
            return bbox_result
        self.visualize(image_with_bbox["image"], bbox_result, self.catid2name,
                       len(self.label_list))
        if os.path.isdir(self.output_dir) is False:
            os.mkdir(self.output_dir)
        bbox_file = os.path.join(self.output_dir, 'bbox.json')
        with open(bbox_file, 'w') as f:
            json.dump(bbox_result, f, indent=4)
        return bbox_result

    def __repr__(self):
        return self.__class__.__name__ + "label_file: {1}, output_dir: {2}".format(
            self.label_file, self.output_dir)


class BlazeFacePostprocess(RCNNPostprocess):
    def clip_bbox(self, bbox, im_size=None):
        h = 1. if im_size is None else im_size[0]
        w = 1. if im_size is None else im_size[1]
        xmin = max(min(bbox[0], w), 0.)
        ymin = max(min(bbox[1], h), 0.)
        xmax = max(min(bbox[2], w), 0.)
        ymax = max(min(bbox[3], h), 0.)
        return xmin, ymin, xmax, ymax

    def _get_bbox_result(self, fetch_map, fetch_name, clsid2catid):
        result = {}
        is_bbox_normalized = True  #for blaze face, set true here
        output = fetch_map[fetch_name]
        lod = [fetch_map[fetch_name + '.lod']]
        lengths = self._offset_to_lengths(lod)
        np_data = np.array(output)
        result['bbox'] = (np_data, lengths)
        result['im_id'] = np.array([[0]])
        result["im_shape"] = np.array(fetch_map["im_shape"]).astype(np.int32)
        bbox_results = self._bbox2out([result], clsid2catid, is_bbox_normalized)
        return bbox_results


class Sequential(object):
    """
    Args:
        sequence (sequence of ``Transform`` objects): list of transforms to chain.

    This API references some of the design pattern of torchvision
    Users can simply use this API in training as well

    Example:
        >>> image_reader.Sequnece([
        >>>     transforms.CenterCrop(10),
        >>> ])
    """

    def __init__(self, transforms):
        self.transforms = transforms

    def __call__(self, img):
        for t in self.transforms:
            img = t(img)
        return img

    def __repr__(self):
        format_string_ = self.__class__.__name__ + '('
        for t in self.transforms:
            format_string_ += '\n'
            format_string_ += '    {0}'.format(t)
        format_string_ += '\n)'
        return format_string_


class DetectionSequential(object):
    """
    Args:
        sequence (sequence of ``Transform`` objects): list of transforms to chain.

    This API references some of the design pattern of torchvision
    Users can simply use this API in training as well

    Example:
        >>> image_reader.Sequnece([
        >>>     transforms.CenterCrop(10),
        >>> ])
    """

    def __init__(self, transforms):
        self.transforms = transforms

    def __call__(self, im):
        im_info = {
        'scale_factor': np.array(
            [1., 1.], dtype=np.float32),
        'im_shape': None,
        }
        for t in self.transforms:
            im, im_info = t(im, im_info)
        return im, im_info

    def __repr__(self):
        format_string_ = self.__class__.__name__ + '('
        for t in self.transforms:
            format_string_ += '\n'
            format_string_ += '    {0}'.format(t)
        format_string_ += '\n)'
        return format_string_


class RGB2BGR(object):
    def __init__(self):
        pass

    def __call__(self, img):
        return img[:, :, ::-1]

    def __repr__(self):
        return self.__class__.__name__ + "()"


class BGR2RGB(object):
    def __init__(self):
        pass

    def __call__(self, img):
        return img[:, :, ::-1]

    def __repr__(self):
        return self.__class__.__name__ + "()"


class DetectionBGR2RGB(object):
    def __init__(self):
        pass

    def __call__(self, img, img_info=None):
        return img[:, :, ::-1], img_info

    def __repr__(self):
        return self.__class__.__name__ + "()"


class String2Image(object):
    def __init__(self):
        pass

    def __call__(self, img_buffer):
        data = np.fromstring(img_buffer, np.uint8)
        img = cv2.imdecode(data, cv2.IMREAD_COLOR)
        return img

    def __repr__(self):
        return self.__class__.__name__ + "()"


class File2Image(object):
    def __init__(self):
        pass

    def __call__(self, img_path):
        if py_version == 2:
            fin = open(img_path)
        else:
            fin = open(img_path, "rb")
        sample = fin.read()
        data = np.fromstring(sample, np.uint8)
        img = cv2.imdecode(data, cv2.IMREAD_COLOR)
        '''
        img = cv2.imread(img_path, -1)
        channels = img.shape[2]
        ori_h = img.shape[0]
        ori_w = img.shape[1]
        '''
        return img

    def __repr__(self):
        return self.__class__.__name__ + "()"

class DetectionFile2Image(object):
    def __init__(self):
        pass

    def __call__(self, img_path, im_info=None):
        if py_version == 2:
            fin = open(img_path)
        else:
            fin = open(img_path, "rb")
        sample = fin.read()
        data = np.fromstring(sample, np.uint8)
        img = cv2.imdecode(data, cv2.IMREAD_COLOR)
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        '''
        img = cv2.imread(img_path, -1)
        channels = img.shape[2]
        ori_h = img.shape[0]
        ori_w = img.shape[1]
        '''
        if im_info is not None:
            im_info['im_shape'] = np.array(img.shape[:2], dtype=np.float32)
            im_info['scale_factor'] = np.array([1., 1.], dtype=np.float32)
        return img, im_info

    def __repr__(self):
        return self.__class__.__name__ + "()"


class URL2Image(object):
    def __init__(self):
        pass

    def __call__(self, img_url):
        resp = urllib.urlopen(img_url)
        sample = resp.read()
        data = np.fromstring(sample, np.uint8)
        img = cv2.imdecode(data, cv2.IMREAD_COLOR)
        return img

    def __repr__(self):
        return self.__class__.__name__ + "()"


class Base64ToImage(object):
    def __init__(self):
        pass

    def __call__(self, img_base64):
        sample = base64.b64decode(img_base64)
        data = np.fromstring(sample, np.uint8)
        img = cv2.imdecode(data, cv2.IMREAD_COLOR)
        return img

    def __repr__(self):
        return self.__class__.__name__ + "()"


class Div(object):
    """ divide by some float number """

    def __init__(self, value):
        self.value = value

    def __call__(self, img):
        """
        Args:
            img (numpy array): (int8 numpy array)

        Returns:
            img (numpy array): (float32 numpy array)
        """
        img = img.astype('float32') / self.value

        return img

    def __repr__(self):
        return self.__class__.__name__ + "({})".format(self.value)

class DetectionDiv(object):
    """ divide by some float number """

    def __init__(self, value):
        self.value = value

    def __call__(self, img, img_info=None):
        """
        Args:
            img (numpy array): (int8 numpy array)

        Returns:
            img (numpy array): (float32 numpy array)
        """
        img = img.astype('float32') / self.value

        return img, img_info

    def __repr__(self):
        return self.__class__.__name__ + "({})".format(self.value)


class Normalize(object):
    """Normalize a tensor image with mean and standard deviation.
    Given mean: ``(M1,...,Mn)`` and std: ``(S1,..,Sn)`` for ``n`` channels, this transform
    will normalize each channel of the input ``torch.*Tensor`` i.e.
    ``output[channel] = (input[channel] - mean[channel]) / std[channel]``

    .. note::
        This transform acts out of place, i.e., it does not mutate the input tensor.

    Args:
        mean (sequence): Sequence of means for each channel.
        std (sequence): Sequence of standard deviations for each channel.

    """

    def __init__(self, mean, std, channel_first=False):
        self.mean = mean
        self.std = std
        self.channel_first = channel_first

    def __call__(self, img):
        """
        Args:
            img (numpy array): (C, H, W) to be normalized.

        Returns:
            Tensor: Normalized Tensor image.
        """
        return F.normalize(img, self.mean, self.std, self.channel_first)

    def __repr__(self):
        return self.__class__.__name__ + '(mean={0}, std={1})'.format(self.mean,
                                                                      self.std)


class DetectionNormalize(object):
    """Normalize a tensor image with mean and standard deviation.
    Given mean: ``(M1,...,Mn)`` and std: ``(S1,..,Sn)`` for ``n`` channels, this transform
    will normalize each channel of the input ``torch.*Tensor`` i.e.
    ``output[channel] = (input[channel] - mean[channel]) / std[channel]``

    .. note::
        This transform acts out of place, i.e., it does not mutate the input tensor.

    Args:
        mean (sequence): Sequence of means for each channel.
        std (sequence): Sequence of standard deviations for each channel.
        is_scale (bool): whether need im / 255

    """

    def __init__(self, mean, std, is_scale=True):
        self.mean = mean
        self.std = std
        self.is_scale = is_scale

    def __call__(self, im, im_info=None):
        """
        Args:
            im (np.ndarray): image (np.ndarray)
            im_info (dict): info of image
        Returns:
            im (np.ndarray):  processed image (np.ndarray)
            im_info (dict): info of processed image
        """
        im = im.astype(np.float32, copy=False)
        mean = np.array(self.mean)[np.newaxis, np.newaxis, :]
        std = np.array(self.std)[np.newaxis, np.newaxis, :]

        if self.is_scale:
            im = im / 255.0
        im -= mean
        im /= std
        return im, im_info

    def __repr__(self):
        return self.__class__.__name__ + '(mean={0}, std={1})'.format(self.mean,
                                                                      self.std)


class Lambda(object):
    """Apply a user-defined lambda as a transform.
       Very shame to just copy from
       https://github.com/pytorch/vision/blob/master/torchvision/transforms/transforms.py#L301

    Args:
        lambd (function): Lambda/function to be used for transform.
    """

    def __init__(self, lambd):
        assert callable(lambd), repr(type(lambd)
                                     .__name__) + " object is not callable"
        self.lambd = lambd

    def __call__(self, img):
        return self.lambd(img)

    def __repr__(self):
        return self.__class__.__name__ + '()'


class CenterCrop(object):
    """Crops the given Image at the center.

    Args:
        size (sequence or int): Desired output size of the crop. If size is an
            int instead of sequence like (h, w), a square crop (size, size) is
            made.
    """

    def __init__(self, size):
        self.size = size

    def __call__(self, img):
        """
        Args:
            img (numpy array): Image to be cropped.

        Returns:
            numpy array Image: Cropped image.
        """
        return F.crop(img, self.size, True)

    def __repr__(self):
        return self.__class__.__name__ + '(size={0})'.format(self.size)


class Resize(object):
    """Resize the input numpy array Image to the given size.

    Args:
        size (sequence or int): Desired output size. If size is a sequence like
            (w, h), output size will be matched to this. If size is an int,
            smaller edge of the image will be matched to this number.
            i.e, if height > width, then image will be rescaled to
            (size * height / width, size)
        interpolation (int, optional): Desired interpolation. Default is
            ``None``
    """

    def __init__(self, size, max_size=2147483647, interpolation=None):
        self.size = size
        self.max_size = max_size
        self.interpolation = interpolation

    def __call__(self, img):
        return F.resize(img, self.size, self.max_size, self.interpolation)

    def __repr__(self):
        return self.__class__.__name__ + '(size={0}, max_size={1}, interpolation={2})'.format(
            self.size, self.max_size,
            _cv2_interpolation_to_str[self.interpolation])

class DetectionResize(object):
    """resize image by target_size and max_size
    Args:
        target_size (int): the target size of image
        keep_ratio (bool): whether keep_ratio or not, default true
        interp (int): method of resize
    """

    def __init__(self, target_size, keep_ratio=True, interpolation=cv2.INTER_LINEAR):
        if isinstance(target_size, int):
            target_size = [target_size, target_size]
        self.target_size = target_size
        self.keep_ratio = keep_ratio
        self.interpolation = interpolation

    def __call__(self, im, im_info=None):
        """
        Args:
            im (np.ndarray): image (np.ndarray)
            im_info (dict): info of image
        Returns:
            im (np.ndarray):  processed image (np.ndarray)
            im_info (dict): info of processed image
        """
        assert len(self.target_size) == 2
        assert self.target_size[0] > 0 and self.target_size[1] > 0
        im_channel = im.shape[2]
        im_scale_y, im_scale_x = self.generate_scale(im)
        im = cv2.resize(
            im,
            None,
            None,
            fx=im_scale_x,
            fy=im_scale_y,
            interpolation=self.interpolation)
        if im_info is not None:
            im_info['im_shape'] = np.array(im.shape[:2]).astype('float32')
            im_info['scale_factor'] = np.array(
                [im_scale_y, im_scale_x]).astype('float32')
        return im, im_info

    def generate_scale(self, im):
        """
        Args:
            im (np.ndarray): image (np.ndarray)
        Returns:
            im_scale_x: the resize ratio of X
            im_scale_y: the resize ratio of Y
        """
        origin_shape = im.shape[:2]
        im_c = im.shape[2]
        if self.keep_ratio:
            im_size_min = np.min(origin_shape)
            im_size_max = np.max(origin_shape)
            target_size_min = np.min(self.target_size)
            target_size_max = np.max(self.target_size)
            im_scale = float(target_size_min) / float(im_size_min)
            if np.round(im_scale * im_size_max) > target_size_max:
                im_scale = float(target_size_max) / float(im_size_max)
            im_scale_x = im_scale
            im_scale_y = im_scale
        else:
            resize_h, resize_w = self.target_size
            im_scale_y = resize_h / float(origin_shape[0])
            im_scale_x = resize_w / float(origin_shape[1])
        return im_scale_y, im_scale_x

    def __repr__(self):
        return self.__class__.__name__ + '(size={0}, max_size={1}, interpolation={2})'.format(
            self.size, self.max_size,
            _cv2_interpolation_to_str[self.interpolation])


class PadStride(object):
    def __init__(self, stride):
        self.coarsest_stride = stride

    def __call__(self, img):
        coarsest_stride = self.coarsest_stride
        if coarsest_stride == 0:
            return img
        im_c, im_h, im_w = img.shape
        pad_h = int(np.ceil(float(im_h) / coarsest_stride) * coarsest_stride)
        pad_w = int(np.ceil(float(im_w) / coarsest_stride) * coarsest_stride)
        padding_im = np.zeros((im_c, pad_h, pad_w), dtype=np.float32)
        padding_im[:, :im_h, :im_w] = img
        im_info = {}
        im_info['resize_shape'] = padding_im.shape[1:]
        return padding_im

class DetectionPadStride(object):
    """ padding image for model with FPN, instead PadBatch(pad_to_stride) in original config
    Args:
        stride (bool): model with FPN need image shape % stride == 0
    """

    def __init__(self, stride=0):
        self.coarsest_stride = stride

    def __call__(self, im, im_info=None):
        """
        Args:
            im (np.ndarray): image (np.ndarray)
            im_info (dict): info of image
        Returns:
            im (np.ndarray):  processed image (np.ndarray)
            im_info (dict): info of processed image
        """
        coarsest_stride = self.coarsest_stride
        if coarsest_stride <= 0:
            return im
        im_c, im_h, im_w = im.shape
        pad_h = int(np.ceil(float(im_h) / coarsest_stride) * coarsest_stride)
        pad_w = int(np.ceil(float(im_w) / coarsest_stride) * coarsest_stride)
        padding_im = np.zeros((im_c, pad_h, pad_w), dtype=np.float32)
        padding_im[:, :im_h, :im_w] = im
        return padding_im, im_info


class ResizeByFactor(object):
    """Resize the input numpy array Image to a size multiple of factor which is usually required by a network

    Args:
        factor (int): Resize factor. make width and height multiple factor of the value of factor. Default is 32
        max_side_len (int): max size of width and height. if width or height is larger than max_side_len, just resize the width or the height. Default is 2400
    """

    def __init__(self, factor=32, max_side_len=2400):
        self.factor = factor
        self.max_side_len = max_side_len

    def __call__(self, img):
        h, w, _ = img.shape
        resize_w = w
        resize_h = h
        if max(resize_h, resize_w) > self.max_side_len:
            if resize_h > resize_w:
                ratio = float(self.max_side_len) / resize_h
            else:
                ratio = float(self.max_side_len) / resize_w
        else:
            ratio = 1.
        resize_h = int(resize_h * ratio)
        resize_w = int(resize_w * ratio)
        if resize_h % self.factor == 0:
            resize_h = resize_h
        elif resize_h // self.factor <= 1:
            resize_h = self.factor
        else:
            resize_h = (resize_h // 32 - 1) * 32
        if resize_w % self.factor == 0:
            resize_w = resize_w
        elif resize_w // self.factor <= 1:
            resize_w = self.factor
        else:
            resize_w = (resize_w // self.factor - 1) * self.factor
        try:
            if int(resize_w) <= 0 or int(resize_h) <= 0:
                return None, (None, None)
            im = cv2.resize(img, (int(resize_w), int(resize_h)))
        except:
            print(resize_w, resize_h)
            sys.exit(0)
        return im

    def __repr__(self):
        return self.__class__.__name__ + '(factor={0}, max_side_len={1})'.format(
            self.factor, self.max_side_len)


class Transpose(object):
    def __init__(self, transpose_target):
        self.transpose_target = transpose_target

    def __call__(self, img):
        return F.transpose(img, self.transpose_target)
        return img

    def __repr__(self):
        format_string = self.__class__.__name__ + \
                        "({})".format(self.transpose_target)
        return format_string

class DetectionTranspose(object):
    def __init__(self, transpose_target):
        self.transpose_target = transpose_target

    def __call__(self, im, im_info=None):
        im = F.transpose(im, self.transpose_target)
        return im, im_info

    def __repr__(self):
        format_string = self.__class__.__name__ + \
                        "({})".format(self.transpose_target)
        return format_string


class SortedBoxes(object):
    """
    Sorted bounding boxes from Detection
    """

    def __init__(self):
        pass

    def __call__(self, dt_boxes):
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


class GetRotateCropImage(object):
    """
    Rotate and Crop image from OCR Det output
    """

    def __init__(self):
        pass

    def __call__(self, img, points):
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


class ImageReader():
    def __init__(self,
                 image_shape=[3, 224, 224],
                 image_mean=[0.485, 0.456, 0.406],
                 image_std=[0.229, 0.224, 0.225],
                 resize_short_size=256,
                 interpolation=None,
                 crop_center=True):
        self.image_mean = image_mean
        self.image_std = image_std
        self.image_shape = image_shape
        self.resize_short_size = resize_short_size
        self.interpolation = interpolation
        self.crop_center = crop_center

    def resize_short(self, img, target_size, interpolation=None):
        """resize image

        Args:
            img: image data
            target_size: resize short target size
            interpolation: interpolation mode

        Returns:
            resized image data
        """
        percent = float(target_size) / min(img.shape[0], img.shape[1])
        resized_width = int(round(img.shape[1] * percent))
        resized_height = int(round(img.shape[0] * percent))
        if interpolation:
            resized = cv2.resize(
                img, (resized_width, resized_height),
                interpolation=interpolation)
        else:
            resized = cv2.resize(img, (resized_width, resized_height))
        return resized

    def crop_image(self, img, target_size, center):
        """crop image

        Args:
            img: images data
            target_size: crop target size
            center: crop mode

        Returns:
            img: cropped image data
        """
        height, width = img.shape[:2]
        size = target_size
        if center == True:
            w_start = (width - size) // 2
            h_start = (height - size) // 2
        else:
            w_start = np.random.randint(0, width - size + 1)
            h_start = np.random.randint(0, height - size + 1)
        w_end = w_start + size
        h_end = h_start + size
        img = img[h_start:h_end, w_start:w_end, :]
        return img

    def process_image(self, sample):
        """ process_image """
        mean = self.image_mean
        std = self.image_std
        crop_size = self.image_shape[1]

        data = np.fromstring(sample, np.uint8)
        img = cv2.imdecode(data, cv2.IMREAD_COLOR)

        if img is None:
            print("img is None, pass it.")
            return None

        if crop_size > 0:
            target_size = self.resize_short_size
            img = self.resize_short(
                img, target_size, interpolation=self.interpolation)
            img = self.crop_image(
                img, target_size=crop_size, center=self.crop_center)

        img = img[:, :, ::-1]

        img = img.astype('float32').transpose((2, 0, 1)) / 255
        img_mean = np.array(mean).reshape((3, 1, 1))
        img_std = np.array(std).reshape((3, 1, 1))
        img -= img_mean
        img /= img_std
        return img
