# Model Zoo

本页面展示了Paddle Serving目前支持的预训练模型以及下载链接
若您想为Paddle Serving提供新的模型，可通过[pull request](https://github.com/PaddlePaddle/Serving/pulls)提交PR

特别感谢[Padddle wholechain](https://www.paddlepaddle.org.cn/wholechain)以及[PaddleHub](https://www.paddlepaddle.org.cn/hub)为Paddle Serving提供的部分预训练模型


| 模型 | 类型 | 部署方式 | 下载 | 服务端 |
| --- | --- | --- | ---- | --- |
| resnet_v2_50_imagenet | PaddleClas | [单模型](../examples/PaddleClas/resnet_v2_50)/[多模型](../examples/pipeline/PaddleClas/ResNet_V2_50) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageClassification/resnet_v2_50_imagenet.tar.gz) | Pipeline Serving, C++ Serving|
| mobilenet_v2_imagenet | PaddleClas | [单模型](../examples/PaddleClas/mobilenet) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageClassification/mobilenet_v2_imagenet.tar.gz) |C++ Serving|
| resnet50_vd | PaddleClas | [单模型](../examples/PaddleClas/imagenet)/[多模型](../examples/pipeline/PaddleClas/ResNet50_vd) | [.tar.gz](https://paddle-serving.bj.bcebos.com/ResNet50_vd.tar) |Pipeline Serving, C++ Serving|
| ResNet50_vd_KL | PaddleClas | [多模型](../examples/pipeline/PaddleClas/ResNet50_vd_KL) | [.tar](https://paddle-serving.bj.bcebos.com/model/ResNet50_vd_KL.tar) |Pipeline Serving|
| DarkNet53 | PaddleClas | [多模型](../examples/pipeline/PaddleClas/DarkNet53) | [.tar](https://paddle-serving.bj.bcebos.com/model/DarkNet53.tar) |Pipeline Serving|
| MobileNetV1 | PaddleClas | [多模型](../examples/pipeline/PaddleClas/MobileNetV1) | [.tar](https://paddle-serving.bj.bcebos.com/model/MobileNetV1.tar) |Pipeline Serving|
| MobileNetV2 | PaddleClas | [多模型](../examples/pipeline/PaddleClas/MobileNetV2) | [.tar](https://paddle-serving.bj.bcebos.com/model/MobileNetV2.tar) |Pipeline Serving|
| MobileNetV3_large_x1_0 | PaddleClas | [多模型](../examples/pipeline/PaddleClas/MobileNetV3_large_x1_0) | [.tar](https://paddle-serving.bj.bcebos.com/model/MobileNetV3_large_x1_0.tar) |Pipeline Serving|
| ResNet50_vd_FPGM | PaddleClas | [多模型](../examples/pipeline/PaddleClas/ResNet50_vd_FPGM) | [.tar](https://paddle-serving.bj.bcebos.com/model/ResNet50_vd_FPGM.tar) |Pipeline Serving|
| ResNet50_vd_PACT | PaddleClas | [多模型](../examples/pipeline/PaddleClas/ResNet50_vd_PACT) | [.tar](https://paddle-serving.bj.bcebos.com/model/ResNet50_vd_PACT.tar) |Pipeline Serving|
| ResNeXt101_vd_64x4d | PaddleClas | [多模型](../examples/pipeline/PaddleClas/ResNeXt101_vd_64x4d) | [.tar](https://paddle-serving.bj.bcebos.com/model/ResNeXt101_vd_64x4d.tar) |Pipeline Serving|
| HRNet_W18_C | PaddleClas | [多模型](../examples/pipeline/PaddleClas/HRNet_W18_C) | [.tar](https://paddle-serving.bj.bcebos.com/model/HRNet_W18_C.tar) |Pipeline Serving|
| ShuffleNetV2_x1_0 | PaddleClas | [多模型](../examples/pipeline/PaddleClas/ShuffleNetV2_x1_0)) | [.tar](https://paddle-serving.bj.bcebos.com/model/ShuffleNetV2_x1_0.tar) |Pipeline Serving|
| bert_chinese_L-12_H-768_A-12 | PaddleNLP | [单模型](../examples/PaddleNLP/bert)/[多模型](../examples/pipeline/bert) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SemanticModel/bert_chinese_L-12_H-768_A-12.tar.gz) |Pipeline Serving, C++ Serving|
| senta_bilstm | PaddleNLP | [单模型](../examples/PaddleNLP/senta) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SentimentAnalysis/senta_bilstm.tar.gz) |C++ Serving|
| lac | PaddleNLP | [单模型](../examples/PaddleNLP/lac) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/LexicalAnalysis/lac.tar.gz) | C++ Serving|
| criteo_ctr | PaddleRec | [单模型](../examples/PaddleRec/criteo_ctr) | [.tar.gz](https://paddle-serving.bj.bcebos.com/criteo_ctr_example/criteo_ctr_demo_model.tar.gz) | C++ Serving |
| criteo_ctr_with_cube | PaddleRec | [单模型](../examples/PaddleRec/criteo_ctr_with_cube) | [.tar.gz](https://paddle-serving.bj.bcebos.com/unittest/ctr_cube_unittest.tar.gz) |C++ Serving|
| blazeface | PaddleDetection | [单模型](../examples/PaddleDetection/blazeface) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ObjectDetection/blazeface.tar.gz) |C++ Serving|
| cascade_mask_rcnn_r50_vd_fpn_ssld_2x_coco | PaddleDetection | [单模型](../examples/PaddleDetection/cascade_rcnn) | [.tar.gz](https://paddle-serving.bj.bcebos.com/pddet_demo/cascade_mask_rcnn_r50_vd_fpn_ssld_2x_coco_serving.tar.gz) |C++ Serving|
| yolov4 | PaddleDetection | [单模型](../examples/PaddleDetection/yolov4) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ObjectDetection/yolov4.tar.gz) |C++ Serving|
| faster_rcnn_hrnetv2p_w18_1x | PaddleDetection | [单模型](../examples/PaddleDetection/faster_rcnn_hrnetv2p_w18_1x) | [.tar.gz](https://paddle-serving.bj.bcebos.com/pddet_demo/faster_rcnn_hrnetv2p_w18_1x.tar.gz) |C++ Serving|
| fcos_dcn_r50_fpn_1x_coco | PaddleDetection | [单模型](../examples/PaddleDetection/fcos_dcn_r50_fpn_1x_coco) | [.tar.gz](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/fcos_dcn_r50_fpn_1x_coco.tar) |C++ Serving|
| ssd_vgg16_300_240e_voc | PaddleDetection |  [单模型](../examples/PaddleDetection/ssd_vgg16_300_240e_voc) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ssd_vgg16_300_240e_voc.tar) |C++ Serving |
| yolov3_darknet53_270e_coco  | PaddleDetection | [单模型](../examples/PaddleDetection/yolov3_darknet53_270e_coco)/[多模型](../examples/pipeline/PaddleDetection/yolov3) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/yolov3_darknet53_270e_coco.tar) |Pipeline Serving, C++ Serving |
| faster_rcnn_r50_fpn_1x_coco | PaddleDetection | [单模型](../examples/PaddleDetection/faster_rcnn_r50_fpn_1x_coco)/[多模型](../examples/pipeline/PaddleDetection/faster_rcnn) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/faster_rcnn_r50_fpn_1x_coco.tar) |Pipeline Serving, C++ Serving |
| ppyolo_r50vd_dcn_1x_coco | PaddleDetection |  [单模型](../examples/PaddleDetection/ppyolo_r50vd_dcn_1x_coco) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_r50vd_dcn_1x_coco.tar) |C++ Serving |
| ppyolo_mbv3_large_coco | PaddleDetection |  [多模型](../examples/pipeline/PaddleDetection/ppyolo_mbv3) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_mbv3_large_coco.tar) |Pipeline Serving |
| ppyolo_r50vd_dcn_1x_coco | PaddleDetection |  [单模型](../examples/PaddleDetection/ppyolo_r50vd_dcn_1x_coco) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_r50vd_dcn_1x_coco.tar) |C++ Serving |
| ttfnet_darknet53_1x_coco | PaddleDetection | [单模型](../examples/PaddleDetection/ttfnet_darknet53_1x_coco) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/ttfnet_darknet53_1x_coco.tar) |C++ Serving |
| ocr_rec | PaddleOCR | [单模型](../examples/PaddleOCR/ocr_rec_det)/[多模型](../examples/pipeline/ocr) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/OCR/ocr_rec.tar.gz) |Pipeline Serving, C++ Serving |
| ocr_det | PaddleOCR | [单模型](../examples/PaddleOCR/ocr_rec_det)/[多模型](../examples/pipeline/ocr) | [.tar.gz](https://paddle-serving.bj.bcebos.com/ocr/ocr_det.tar.gz) |Pipeline Serving, C++ Serving |
| deeplabv3 | PaddleSeg | [单模型](../examples/PaddleSeg/deeplabv3) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageSegmentation/deeplabv3.tar.gz) | C++ Serving |
| unet | PaddleSeg | [单模型](../examples/PaddleSeg/unet_for_image_seg) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageSegmentation/unet.tar.gz) |C++ Serving |

- 注意事项
  - 多模型部署示例均在pipeline文件夹下
  - 单模型采用C++ Serving，多模型采用Pipeline Serving

- 请参考 [example](../examples) 查看详情

- 更多模型请参考[wholechain](https://www.paddlepaddle.org.cn/wholechain)

