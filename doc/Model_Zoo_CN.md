# Model Zoo

本页面展示了Paddle Serving目前支持的预训练模型以及下载链接
若您想为Paddle Serving提供新的模型，可通过[pull request](https://github.com/PaddlePaddle/Serving/pulls)提交PR

特别感谢[Padddle wholechain](https://www.paddlepaddle.org.cn/wholechain)以及[PaddleHub](https://www.paddlepaddle.org.cn/hub)为Paddle Serving提供的部分预训练模型


| 模型 | 类型 | 部署方式 | 下载 | 服务端启动方式 |
| --- | --- | --- | ---- | --- |
| resnet_v2_50_imagenet | PaddleClas | [单模型](../examples/PaddleClas/resnet_v2_50) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageClassification/resnet_v2_50_imagenet.tar.gz) |C++|
| mobilenet_v2_imagenet | PaddleClas | [单模型](../examples/PaddleClas/mobilenet) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageClassification/mobilenet_v2_imagenet.tar.gz) |C++|
| resnet50_vd | PaddleClas | [单模型](../examples/PaddleClas/imagenet)/[多模型](../examples/pipeline/imagenet) | [.tar.gz](https://paddle-serving.bj.bcebos.com/imagenet-example/ResNet50_vd.tar.gz) |python, C++|
| bert_chinese_L-12_H-768_A-12 | PaddleNLP | [单模型](../examples/PaddleNLP/bert)/[多模型](../examples/pipeline/bert) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SemanticModel/bert_chinese_L-12_H-768_A-12.tar.gz) |python, C++|
| senta_bilstm | PaddleNLP | [单模型](../examples/PaddleNLP/senta) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SentimentAnalysis/senta_bilstm.tar.gz) |C++|
| lac | PaddleNLP | [单模型](../examples/PaddleNLP/lac) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/LexicalAnalysis/lac.tar.gz) |C++|
| criteo_ctr | PaddleRec | [单模型](../examples/PaddleRec/criteo_ctr) | [.tar.gz](https://paddle-serving.bj.bcebos.com/criteo_ctr_example/criteo_ctr_demo_model.tar.gz) | C++ |
| criteo_ctr_with_cube | PaddleRec | [单模型](../examples/PaddleRec/criteo_ctr_with_cube) | [.tar.gz](https://paddle-serving.bj.bcebos.com/unittest/ctr_cube_unittest.tar.gz) |C++|
| blazeface | PaddleDetection | [单模型](../examples/PaddleDetection/blazeface) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ObjectDetection/blazeface.tar.gz) |C++|
| cascade_mask_rcnn_r50_vd_fpn_ssld_2x_coco_serving | PaddleDetection | [单模型](../examples/PaddleDetection/cascade_rcnn) | [.tar.gz](https://paddle-serving.bj.bcebos.com/pddet_demo/cascade_mask_rcnn_r50_vd_fpn_ssld_2x_coco_serving.tar.gz) |C++|
| yolov4 | PaddleDetection | [单模型](../examples/PaddleDetection/yolov4) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ObjectDetection/yolov4.tar.gz) |C++|
| faster_rcnn_hrnetv2p_w18_1x | PaddleDetection | [单模型](../examples/PaddleDetection/faster_rcnn_hrnetv2p_w18_1x) | [.tar.gz](https://paddle-serving.bj.bcebos.com/pddet_demo/faster_rcnn_hrnetv2p_w18_1x.tar.gz) |C++|
| fcos_dcn_r50_fpn_1x_coco | PaddleDetection | [单模型](../examples/PaddleDetection/fcos_dcn_r50_fpn_1x_coco) | [.tar.gz](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/fcos_dcn_r50_fpn_1x_coco.tar) |C++|
| ssd_vgg16_300_240e_voc | PaddleDetection |  [单模型](../examples/PaddleDetection/ssd_vgg16_300_240e_voc) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ssd_vgg16_300_240e_voc.tar) |C++ |
| yolov3_darknet53_270e_coco  | PaddleDetection | [单模型](../examples/PaddleDetection/yolov3_darknet53_270e_coco)/[多模型](../examples/pipeline/PaddleDetection/yolov3) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/yolov3_darknet53_270e_coco.tar) |python, C++ |
| faster_rcnn_r50_fpn_1x_coco | PaddleDetection | [单模型](../examples/PaddleDetection/faster_rcnn_r50_fpn_1x_coco)/[多模型](../examples/pipeline/PaddleDetection/faster_rcnn) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/faster_rcnn_r50_fpn_1x_coco.tar) |python, C++ |
| ppyolo_r50vd_dcn_1x_coco | PaddleDetection |  [单模型](../examples/PaddleDetection/ppyolo_r50vd_dcn_1x_coco) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_r50vd_dcn_1x_coco.tar) |C++ |
| ppyolo_mbv3_large_coco | PaddleDetection |  [多模型](../examples/pipeline/PaddleDetection/ppyolo_mbv3) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_mbv3_large_coco.tar) |python |
| ppyolo_r50vd_dcn_1x_coco | PaddleDetection |  [单模型](../examples/PaddleDetection/ppyolo_r50vd_dcn_1x_coco) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_r50vd_dcn_1x_coco.tar) |C++ |
| ttfnet_darknet53_1x_coco | PaddleDetection | [单模型](../examples/PaddleDetection/ttfnet_darknet53_1x_coco) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/ttfnet_darknet53_1x_coco.tar) |C++ |
| ocr_rec | PaddleOCR | [单模型](../examples/PaddleOCR/ocr_rec_det)/[多模型](../examples/pipeline/ocr) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/OCR/ocr_rec.tar.gz) |python, C++ |
| ocr_det | PaddleOCR | [单模型](../examples/PaddleOCR/ocr_rec_det)/[多模型](../examples/pipeline/ocr) | [.tar.gz](https://paddle-serving.bj.bcebos.com/ocr/ocr_det.tar.gz) |python, C++ |
| deeplabv3 | PaddleSeg | [单模型](../examples/PaddleSeg/deeplabv3) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageSegmentation/deeplabv3.tar.gz) | C++ |
| unet | PaddleSeg | [单模型](../examples/PaddleSeg/unet_for_image_seg) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageSegmentation/unet.tar.gz) |C++ |

请参考 [example](../examples) 查看详情

更多模型请参考[wholechain](https://www.paddlepaddle.org.cn/wholechain)

