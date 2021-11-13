## 使用Paddle Detection模型

([English](./README.md)|简体中文)

### 简介

PaddleDetection飞桨目标检测开发套件，旨在帮助开发者更快更好地完成检测模型的组建、训练、优化及部署等全开发流程。详情参见[Github](https://github.com/PaddlePaddle/PaddleDetection/tree/master)

本文主要是介绍Paddle Detection的动态图模型在Serving上的部署。

### 导出模型

Paddle Detection提供了大量的[模型库](https://github.com/PaddlePaddle/PaddleDetection/blob/master/docs/MODEL_ZOO_cn.md), 这些模型库配合导出工具都可以得到可以用于Paddle Serving的模型。导出教程参见[Paddle Detection模型导出教程](https://github.com/PaddlePaddle/PaddleDetection/blob/master/deploy/EXPORT_MODEL.md)。

### Serving示例
本文件夹下给出了多个PaddleDetection模型用于Serving的范例

- [Faster RCNN](./faster_rcnn_r50_fpn_1x_coco)
- [PPYOLO](./ppyolo_r50vd_dcn_1x_coco)
- [TTFNet](./ttfnet_darknet53_1x_coco)
- [YOLOv3](./yolov3_darknet53_270e_coco)
- [HRNet](./faster_rcnn_hrnetv2p_w18_1x)
- [Fcos](./fcos_dcn_r50_fpn_1x_coco)
- [SSD](./ssd_vgg16_300_240e_voc/)
