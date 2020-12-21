# coding=utf-8
import base64
import json
import math
import os
import time
from collections import OrderedDict
import cv2
import numpy as np
from PIL import Image, ImageDraw
from paddle_serving_client import Client


class DetectFace:
    def __init__(self, output_dir):
        self.mask = Mask()
        self.output_dir = output_dir

    def base64_to_cv2(self, b64str):
        data = base64.b64decode(b64str.encode('utf8'))
        data = np.fromstring(data, np.uint8)
        data = cv2.imdecode(data, cv2.IMREAD_COLOR)
        return data

    def check_dir(self, dir_path):
        """
        Create directory to save processed image.

        Args:
            dir_path (str): directory path to save images.

        """
        if not os.path.exists(dir_path):
            os.makedirs(dir_path)
        elif os.path.isfile(dir_path):
            os.remove(dir_path)
            os.makedirs(dir_path)

    def get_save_image_name(self, org_im, org_im_path, output_dir):
        """
        Get save image name from source image path.
        """
        # name prefix of original image
        org_im_name = os.path.split(org_im_path)[-1]
        im_prefix = os.path.splitext(org_im_name)[0]
        # extension
        img = Image.fromarray(org_im[:, :, ::-1])
        if img.mode == 'RGBA':
            ext = '.png'
        elif img.mode == 'RGB':
            ext = '.jpg'
        # save image path
        save_im_path = os.path.join(output_dir, im_prefix + ext)
        if os.path.exists(save_im_path):
            save_im_path = os.path.join(
                output_dir, im_prefix + ext)
        return save_im_path

    def clip_bbox(self, bbox, img_height, img_width):
        bbox['left'] = int(max(min(bbox['left'], img_width), 0.))
        bbox['top'] = int(max(min(bbox['top'], img_height), 0.))
        bbox['right'] = int(max(min(bbox['right'], img_width), 0.))
        bbox['bottom'] = int(max(min(bbox['bottom'], img_height), 0.))
        return bbox

    def preprocess(self, im_path, shrink=0.5):
        im = cv2.imread(im_path)
        image = im.copy()
        image_height, image_width, image_channel = image.shape
        if shrink != 1:
            image_height, image_width = int(image_height * shrink), int(
                image_width * shrink)
            image = cv2.resize(image, (image_width, image_height),
                               cv2.INTER_NEAREST)
        # HWC to CHW
        if len(image.shape) == 3:
            image = np.swapaxes(image, 1, 2)
            image = np.swapaxes(image, 1, 0)
        # mean, std
        mean = [104., 117., 123.]
        scale = 0.007843
        image = image.astype('float32')
        image -= np.array(mean)[:, np.newaxis, np.newaxis].astype('float32')
        image = image * scale
        return image, image_height, image_width

    def postprocess(self, image_with_bbox, visualization=True, shrink=0.5, confs_threshold=0.6):
        org_im_path = image_with_bbox['image']
        org_im = cv2.imread(org_im_path)
        _, image_height, image_width = image_with_bbox['im_shape']
        out_data = image_with_bbox['save_infer_model/scale_0']
        output = dict()
        output['data'] = list()
        output['path'] = org_im_path
        for each_data in out_data:
            # each_data is a list: [label, confidence, left, top, right, bottom]
            if each_data[1] > confs_threshold:
                dt_bbox = dict()
                dt_bbox['confidence'] = float(each_data[1])
                dt_bbox['left'] = image_width * each_data[2] / shrink
                dt_bbox['top'] = image_height * each_data[3] / shrink
                dt_bbox['right'] = image_width * each_data[4] / shrink
                dt_bbox['bottom'] = image_height * each_data[5] / shrink
                dt_bbox = self.clip_bbox(dt_bbox, org_im.shape[0], org_im.shape[1])
                output['data'].append(dt_bbox)
        bbox_file = os.path.join(self.output_dir, 'bbox.json')
        with open(bbox_file, 'w') as f:
            json.dump(output, f, indent=4)
        if visualization:
            self.check_dir(self.output_dir)
            save_im_path = self.get_save_image_name(org_im, org_im_path, self.output_dir)
            im_out = org_im.copy()
            if len(output['data']) > 0:
                output_data = list()
                for bbox in output['data']:
                    face = {'left': bbox['left'], 'top': bbox['top'], 'right': bbox['right'], 'bottom': bbox['bottom']}
                    image_in = self.mask.preprocess(im_out, face)
                    fetch_map = self.mask.client.predict(
                        feed={"image": image_in}, fetch=["save_infer_model/scale_0"])
                    text, confidence = self.mask.postprocess(fetch_map)
                    face['label'] = text
                    face['confidence'] = confidence
                    output_data.append(face)
                cv2.imwrite(save_im_path, im_out)
                self.draw_bounding_box_on_image(save_im_path, output_data)

    def draw_bounding_box_on_image(self, save_im_path, output_data):
        image = Image.open(save_im_path)
        draw = ImageDraw.Draw(image)
        for bbox in output_data:
            # draw bouding box
            if bbox['label'] == "MASK":
                draw.line([(bbox['left'], bbox['top']),
                           (bbox['left'], bbox['bottom']),
                           (bbox['right'], bbox['bottom']),
                           (bbox['right'], bbox['top']),
                           (bbox['left'], bbox['top'])],
                          width=2,
                          fill='green')
            else:
                draw.line([(bbox['left'], bbox['top']),
                           (bbox['left'], bbox['bottom']),
                           (bbox['right'], bbox['bottom']),
                           (bbox['right'], bbox['top']),
                           (bbox['left'], bbox['top'])],
                          width=2,
                          fill='red')
            # draw label
            text = bbox['label'] + ": %.2f%%" % (100 * bbox['confidence'])
            textsize_width, textsize_height = draw.textsize(text=text)
            if image.mode == 'RGB' or image.mode == 'RGBA':
                box_fill = (255, 255, 255)
                text_fill = (0, 0, 0)
            else:
                box_fill = (255)
                text_fill = (0)
            draw.rectangle(
                xy=(bbox['left'], bbox['top'] - (textsize_height + 5),
                    bbox['left'] + textsize_width + 10, bbox['top'] - 3),
                fill=box_fill)
            draw.text(
                xy=(bbox['left'], bbox['top'] - 15), text=text, fill=text_fill)
        image.save(save_im_path)


class Mask:
    def __init__(self):
        self.client = Client()
        self.client.load_client_config('mask_serving_client/serving_client_conf.prototxt')
        self.client.connect(['127.0.0.1:9495'])

    def color_normalize(self, image, mean):
        if image.shape[-1] == 1:
            image = np.repeat(image, axis=2)
        h, w, c = image.shape
        image = np.transpose(image, (2, 0, 1))
        image = np.subtract(image.reshape(c, -1), mean[:, np.newaxis]).reshape(
            -1, h, w)
        image = np.transpose(image, (1, 2, 0))
        return image

    def crop(self, image, pts, shift=0, scale=1.5, rotate=0, res_width=128, res_height=128):
        res = (res_width, res_height)
        idx1 = 0
        idx2 = 1
        # angle
        alpha = 0
        if pts[idx2, 0] != -1 and pts[idx2, 1] != -1 and pts[idx1, 0] != -1 and pts[
            idx1, 1] != -1:
            alpha = math.atan2(pts[idx2, 1] - pts[idx1, 1],
                               pts[idx2, 0] - pts[idx1, 0]) * 180 / math.pi
        pts[pts == -1] = np.inf
        coord_min = np.min(pts, 0)
        pts[pts == np.inf] = -1
        coord_max = np.max(pts, 0)
        # coordinates of center point
        c = np.array([
            coord_max[0] - (coord_max[0] - coord_min[0]) / 2,
            coord_max[1] - (coord_max[1] - coord_min[1]) / 2
        ])  # center
        max_wh = max((coord_max[0] - coord_min[0]) / 2,
                     (coord_max[1] - coord_min[1]) / 2)
        # Shift the center point, rot add eyes angle
        c = c + shift * max_wh
        rotate = rotate + alpha
        M = cv2.getRotationMatrix2D((c[0], c[1]), rotate,
                                    res[0] / (2 * max_wh * scale))
        M[0, 2] = M[0, 2] - (c[0] - res[0] / 2.0)
        M[1, 2] = M[1, 2] - (c[1] - res[0] / 2.0)
        image_out = cv2.warpAffine(image, M, res)
        return image_out, M

    def preprocess(self, org_im, face):
        pts = np.array([
            face['left'], face['top'], face['right'], face['top'], face['left'],
            face['bottom'], face['right'], face['bottom']
        ]).reshape(4, 2).astype(np.float32)
        image_in, M = self.crop(org_im, pts)
        image_in = image_in / 256.0
        image_in = self.color_normalize(image_in, mean=np.array([0.5, 0.5, 0.5]))
        image_in = image_in.astype(np.float32).transpose([2, 0, 1]).reshape(3, 128, 128)
        return image_in

    def postprocess(self, fetch_map):
        classid = np.argmax(fetch_map['save_infer_model/scale_0'])
        text = 'MASK' if classid == 1 else 'No MASK'
        confidence = fetch_map['save_infer_model/scale_0'][0][classid]
        return text, confidence
