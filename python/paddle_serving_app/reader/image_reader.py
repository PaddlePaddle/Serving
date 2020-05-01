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
import urllib
import numpy as np
import base64
import functional as F

_cv2_interpolation_to_str = {cv2.INTER_LINEAR: "cv2.INTER_LINEAR"}


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


class File2Image(object):
    def __init__(self):
        pass

    def __call__(self, img_path):
        fin = open(img_path)
        sample = fin.read()
        data = np.fromstring(sample, np.uint8)
        img = cv2.imdecode(data, cv2.IMREAD_COLOR)
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

    def __init__(self, mean, std):
        self.mean = mean
        self.std = std

    def __call__(self, img):
        """
        Args:
            img (numpy array): (C, H, W) to be normalized.

        Returns:
            Tensor: Normalized Tensor image.
        """
        return F.normalize(img, self.mean, self.std)

    def __repr__(self):
        return self.__class__.__name__ + '(mean={0}, std={1})'.format(self.mean,
                                                                      self.std)


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
            ``PIL.Image.BILINEAR``
    """

    def __init__(self, size, interpolation=cv2.INTER_LINEAR):
        self.size = size
        self.interpolation = interpolation

    def __call__(self, img):
        return F.resize(img, self.size, self.interpolation)

    def __repr__(self, img):
        return self.__class__.__name__ + '(size={0}, interpolation={1})'.format(
            self.size, _cv2_interpolation_to_str[self.interpolation])


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
