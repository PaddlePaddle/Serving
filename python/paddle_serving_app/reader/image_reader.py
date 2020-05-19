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
import os
import urllib
import numpy as np
import base64
from . import functional as F
from PIL import Image, ImageDraw
import json

_cv2_interpolation_to_str = {cv2.INTER_LINEAR: "cv2.INTER_LINEAR", None: "None"}


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
            result_png,
            ori_shape[:2],
            fx=0,
            fy=0,
            interpolation=cv2.INTER_CUBIC)
        cv2.imwrite(vis_result_name, result_png, [cv2.CV_8UC1])


class RCNNPostprocess(object):
    def __init__(self, label_file, output_dir):
        self.output_dir = output_dir
        self.label_file = label_file
        self.label_list = []
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
                        im_shape = t['im_shape'][0][i].tolist()
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

    def __call__(self, image_with_bbox):
        fetch_name = ""
        for key in image_with_bbox:
            if key == "image":
                continue
            if ".lod" in key:
                continue
            fetch_name = key
        bbox_result = self._get_bbox_result(image_with_bbox, fetch_name,
                                            self.clsid2catid)
        if os.path.isdir(self.output_dir) is False:
            os.mkdir(self.output_dir)
        self.visualize(image_with_bbox["image"], bbox_result, self.catid2name,
                       len(self.label_list))
        if os.path.isdir(self.output_dir) is False:
            os.mkdir(self.output_dir)
        bbox_file = os.path.join(self.output_dir, 'bbox.json')
        with open(bbox_file, 'w') as f:
            json.dump(bbox_result, f, indent=4)

    def __repr__(self):
        return self.__class__.__name__ + "label_file: {1}, output_dir: {2}".format(
            self.label_file, self.output_dir)


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


class File2Image(object):
    def __init__(self):
        pass

    def __call__(self, img_path):
        fin = open(img_path)
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
        img = base64.b64decode(img_base64)
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
            (h, w), output size will be matched to this. If size is an int,
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
