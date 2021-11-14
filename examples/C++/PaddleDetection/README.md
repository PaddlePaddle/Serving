# Serve models from Paddle Detection

(English|[简体中文](./README_CN.md))

### Introduction

PaddleDetection flying paddle target detection development kit is designed to help developers complete the whole development process of detection model formation, training, optimization and deployment faster and better. For details, see [Github](https://github.com/PaddlePaddle/PaddleDetection/tree/master)

This article mainly introduces the deployment of Paddle Detection's dynamic graph model on Serving.

Paddle Detection provides a large number of [Model Zoo](https://github.com/PaddlePaddle/PaddleDetection/blob/master/docs/MODEL_ZOO_cn.md), these model libraries can be used in Paddle Serving with export tools Model. For the export tutorial, please refer to [Paddle Detection Export Model Tutorial (Simplified Chinese)](https://github.com/PaddlePaddle/PaddleDetection/blob/master/deploy/EXPORT_MODEL.md).

### Serving example
Several examples of PaddleDetection models used in Serving are given in this folder
All examples support TensorRT.

- [Faster RCNN](./faster_rcnn_r50_fpn_1x_coco)
- [PPYOLO](./ppyolo_r50vd_dcn_1x_coco)
- [TTFNet](./ttfnet_darknet53_1x_coco)
- [YOLOv3](./yolov3_darknet53_270e_coco)
- [HRNet](./faster_rcnn_hrnetv2p_w18_1x)
- [Fcos](./fcos_dcn_r50_fpn_1x_coco)
- [SSD](./ssd_vgg16_300_240e_voc/)
