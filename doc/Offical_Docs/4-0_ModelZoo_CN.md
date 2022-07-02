# 模型库

- [模型分类](#1)
  - [1.1 图像分类与识别](#1.1)
  - [1.2 文本类](#1.2)
  - [1.3 推荐系统](#1.3)
  - [1.4 人脸识别](#1.4)
  - [1.5 目标检测](#1.5)
  - [1.6 文字识别](#1.6)
  - [1.7 图像分割](#1.7)
  - [1.8 关键点检测](#1.8)
  - [1.9 视频理解](#1.9)
- [模型示例库](#2)

Paddle Serving 已实现9个类别，共计46个模型的服务化部署示例。

<a name="1"></a>

## 模型分类

<a name="1.1"></a>

**一.图像分类与识别**

模型部署示例请参阅下表：
| 场景| 模型 | 类型 | 示例使用的框架 | 下载 |
| --- | --- | --- | --- | ---- |
| 图像识别 |pp_shitu | PaddleClas | [C++ Serving](../examples/C++/PaddleClas/pp_shitu) | [.tar.gz](https://paddle-imagenet-models-name.bj.bcebos.com/dygraph/rec/models/inference/serving/pp_shitu.tar.gz) |
| 图像分类 | resnet_v2_50_imagenet | PaddleClas | [C++ Serving](../examples/C++/PaddleClas/resnet_v2_50)</br>[Pipeline Serving](../examples/Pipeline/PaddleClas/ResNet_V2_50) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageClassification/resnet_v2_50_imagenet.tar.gz) | Pipeline Serving, C++ Serving|
| 图像分类 |mobilenet_v2_imagenet | PaddleClas | [C++ Serving](../examples/C++/PaddleClas/mobilenet) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageClassification/mobilenet_v2_imagenet.tar.gz) |
| 图像分类 |resnet50_vd | PaddleClas | [C++ Serving](../examples/C++/PaddleClas/imagenet)</br>[Pipeline Serving](../examples/Pipeline/PaddleClas/ResNet50_vd) | [.tar.gz](https://paddle-serving.bj.bcebos.com/model/ResNet50_vd.tar) |
| 图像分类 |ResNet50_vd_KL | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/ResNet50_vd_KL) | [.tar](https://paddle-serving.bj.bcebos.com/model/ResNet50_vd_KL.tar) |
| 图像分类 |ResNet50_vd_FPGM | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/ResNet50_vd_FPGM) | [.tar](https://paddle-serving.bj.bcebos.com/model/ResNet50_vd_FPGM.tar) |
| 图像分类 |ResNet50_vd_PACT | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/ResNet50_vd_PACT) | [.tar](https://paddle-serving.bj.bcebos.com/model/ResNet50_vd_PACT.tar) |
| 图像分类 |ResNeXt101_vd_64x4d | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/ResNeXt101_vd_64x4d) | [.tar](https://paddle-serving.bj.bcebos.com/model/ResNeXt101_vd_64x4d.tar) |
| 图像分类 |DarkNet53 | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/DarkNet53) | [.tar](https://paddle-serving.bj.bcebos.com/model/DarkNet53.tar) |
| 图像分类 |MobileNetV1 | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/MobileNetV1) | [.tar](https://paddle-serving.bj.bcebos.com/model/MobileNetV1.tar) |
| 图像分类 |MobileNetV2 | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/MobileNetV2) | [.tar](https://paddle-serving.bj.bcebos.com/model/MobileNetV2.tar) |
| 图像分类 |MobileNetV3_large_x1_0 | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/MobileNetV3_large_x1_0) | [.tar](https://paddle-serving.bj.bcebos.com/model/MobileNetV3_large_x1_0.tar) |
| 图像生成 |HRNet_W18_C | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/HRNet_W18_C) | [.tar](https://paddle-serving.bj.bcebos.com/model/HRNet_W18_C.tar) |
| 图像分类 |ShuffleNetV2_x1_0 | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/ShuffleNetV2_x1_0) | [.tar](https://paddle-serving.bj.bcebos.com/model/ShuffleNetV2_x1_0.tar) |

---

<a name="1.2"></a>

**二.文本类**

模型部署示例请参阅下表：
| 场景| 模型 | 类型 | 示例使用的框架 | 下载 |
| --- | --- | --- | --- | ---- |
| 文本生成 | bert_chinese_L-12_H-768_A-12 | PaddleNLP | [C++ Serving](../examples/C++/PaddleNLP/bert)</br>[Pipeline Serving](../examples/Pipeline/PaddleNLP/bert) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SemanticModel/bert_chinese_L-12_H-768_A-12.tar.gz) |
| 情感分析 |senta_bilstm | PaddleNLP | [C++ Serving](../examples/C++/PaddleNLP/senta) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SentimentAnalysis/senta_bilstm.tar.gz) |C++ Serving|
| 词法分析 |lac | PaddleNLP | [C++ Serving](../examples/C++/PaddleNLP/lac) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/LexicalAnalysis/lac.tar.gz) |
| 机器翻译 |transformer | PaddleNLP | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleNLP/blob/develop/examples/machine_translation/transformer/deploy/serving/README.md) | [model](https://github.com/PaddlePaddle/PaddleNLP/tree/develop/examples/machine_translation/transformer) |
| 标点符号预测 | ELECTRA | PaddleNLP | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleNLP/blob/develop/examples/language_model/electra/deploy/serving/README.md) | [model](https://github.com/PaddlePaddle/PaddleNLP/tree/develop/examples/language_model/electra) |
| 抽取文本向量| In-batch Negatives | PaddleNLP | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleNLP/tree/develop/applications/neural_search/recall/in_batch_negative) | [model](https://bj.bcebos.com/v1/paddlenlp/models/inbatch_model.zip) |

---
<a name="1.3"></a>

**三.推荐系统**

模型部署示例请参阅下表：
| 场景| 模型 | 类型 | 示例使用的框架 | 下载 |
| --- | --- | --- | --- | ---- |
| CTR预估 | criteo_ctr | PaddleRec | [C++ Serving](../examples/C++/PaddleRec/criteo_ctr) | [.tar.gz](https://paddle-serving.bj.bcebos.com/criteo_ctr_example/criteo_ctr_demo_model.tar.gz) |
| CTR预估 | criteo_ctr_with_cube | PaddleRec | [C++ Serving](../examples/C++/PaddleRec/criteo_ctr_with_cube) | [.tar.gz](https://paddle-serving.bj.bcebos.com/unittest/ctr_cube_unittest.tar.gz) |
| 内容推荐 | wide&deep | PaddleRec | [C++ Serving](https://github.com/PaddlePaddle/PaddleRec/blob/release/2.1.0/doc/serving.md) | [model](https://github.com/PaddlePaddle/PaddleRec/blob/release/2.1.0/models/rank/wide_deep/README.md) |

---
<a name="1.4"></a>

**四.人脸识别**

模型部署示例请参阅下表：
| 场景| 模型 | 类型 | 示例使用的框架 | 下载 |
| --- | --- | --- | --- | ---- |
| 人脸识别|blazeface | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/blazeface) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ObjectDetection/blazeface.tar.gz) |C++ Serving|

---
<a name="1.5"></a>

**五.目标检测**

模型部署示例请参阅下表：
| 场景| 模型 | 类型 | 示例使用的框架 | 下载 |
| --- | --- | --- | --- | ---- |
| 目标检测 |cascade_mask_rcnn_r50_vd_fpn_ssld_2x_coco | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/cascade_rcnn) | [.tar.gz](https://paddle-serving.bj.bcebos.com/pddet_demo/cascade_mask_rcnn_r50_vd_fpn_ssld_2x_coco_serving.tar.gz) |
| 目标检测 | yolov4 | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/yolov4) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ObjectDetection/yolov4.tar.gz) |C++ Serving|
| 目标检测 |fcos_dcn_r50_fpn_1x_coco | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/fcos_dcn_r50_fpn_1x_coco) | [.tar.gz](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/fcos_dcn_r50_fpn_1x_coco.tar) |
| 目标检测 | ssd_vgg16_300_240e_voc | PaddleDetection |  [C++ Serving](../examples/C++/PaddleDetection/ssd_vgg16_300_240e_voc) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ssd_vgg16_300_240e_voc.tar) |
| 目标检测 |yolov3_darknet53_270e_coco  | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/yolov3_darknet53_270e_coco)</br>[Pipeline Serving](../examples/Pipeline/PaddleDetection/yolov3) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/yolov3_darknet53_270e_coco.tar) |
| 目标检测 | faster_rcnn_r50_fpn_1x_coco | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/faster_rcnn_r50_fpn_1x_coco)</br>[Pipeline Serving](../examples/Pipeline/PaddleDetection/faster_rcnn) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/faster_rcnn_r50_fpn_1x_coco.tar) |
| 目标检测 |ppyolo_r50vd_dcn_1x_coco | PaddleDetection |  [C++ Serving](../examples/C++/PaddleDetection/ppyolo_r50vd_dcn_1x_coco) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_r50vd_dcn_1x_coco.tar) |
| 目标检测 | ppyolo_mbv3_large_coco | PaddleDetection |  [Pipeline Serving](../examples/Pipeline/PaddleDetection/ppyolo_mbv3) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_mbv3_large_coco.tar) |
| 目标检测 | ttfnet_darknet53_1x_coco | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/ttfnet_darknet53_1x_coco) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/ttfnet_darknet53_1x_coco.tar) |
| 目标检测 |YOLOv3-DarkNet | PaddleDetection | [C++ Serving](https://github.com/PaddlePaddle/PaddleDetection/tree/release/2.3/deploy/serving) | [.pdparams](https://paddledet.bj.bcebos.com/models/yolov3_darknet53_270e_coco.pdparams)</br>[.yml](https://github.com/PaddlePaddle/PaddleDetection/blob/develop/configs/yolov3/yolov3_darknet53_270e_coco.yml) |

---
<a name="1.6"></a>

**六.文字识别**

模型部署示例请参阅下表：
| 场景| 模型 | 类型 | 示例使用的框架 | 下载 |
| --- | --- | --- | --- | ---- |
| 文字识别 |ocr_rec | PaddleOCR | [C++ Serving](../examples/C++/PaddleOCR/ocr)</br>[Pipeline Serving](../examples/Pipeline/PaddleOCR/ocr) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/OCR/ocr_rec.tar.gz) |
| 文字识别 |ocr_det | PaddleOCR | [C++ Serving](../examples/C++/PaddleOCR/ocr)</br>[Pipeline Serving](../examples/Pipeline/PaddleOCR/ocr) | [.tar.gz](https://paddle-serving.bj.bcebos.com/ocr/ocr_det.tar.gz) |
| 文字识别 |ch_ppocr_mobile_v2.0_det | PaddleOCR | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/deploy/pdserving/README.md) | [model](https://paddleocr.bj.bcebos.com/dygraph_v2.0/ch/ch_ppocr_mobile_v2.0_det_infer.tar)</br>[.yml](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/configs/det/ch_ppocr_v2.0/ch_det_mv3_db_v2.0.yml) |
| 文字识别 |ch_ppocr_server_v2.0_det | PaddleOCR | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/deploy/pdserving/README.md) | [model](https://paddleocr.bj.bcebos.com/dygraph_v2.0/ch/ch_ppocr_server_v2.0_det_infer.tar)</br>[.yml](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/configs/det/ch_ppocr_v2.0/ch_det_res18_db_v2.0.yml) |
| 文字识别 |ch_ppocr_mobile_v2.0_rec | PaddleOCR | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/deploy/pdserving/README.md) | [model](https://paddleocr.bj.bcebos.com/dygraph_v2.0/ch/ch_ppocr_mobile_v2.0_rec_infer.tar)</br>[.yml](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/configs/rec/ch_ppocr_v2.0/rec_chinese_lite_train_v2.0.yml) |
| 文字识别 |ch_ppocr_server_v2.0_rec | PaddleOCR | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/deploy/pdserving/README.md) | [model](https://paddleocr.bj.bcebos.com/dygraph_v2.0/ch/ch_ppocr_server_v2.0_rec_infer.tar)</br>[.yml](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/configs/rec/ch_ppocr_v2.0/rec_chinese_common_train_v2.0.yml) |
| 文字识别 |ch_ppocr_mobile_v2.0 | PaddleOCR | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/deploy/pdserving/README.md) | [model](https://github.com/PaddlePaddle/PaddleOCR) |
| 文字识别 |ch_ppocr_server_v2.0 | PaddleOCR | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/deploy/pdserving/README.md) | [model](https://github.com/PaddlePaddle/PaddleOCR) |

---
<a name="1.7"></a>

**七.图像分割**

模型部署示例请参阅下表：
| 场景| 模型 | 类型 | 示例使用的框架 | 下载 |
| --- | --- | --- | --- | ---- |
| 图像分割 | deeplabv3 | PaddleSeg | [C++ Serving](../examples/C++/PaddleSeg/deeplabv3) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageSegmentation/deeplabv3.tar.gz) |
| 图像分割 | unet | PaddleSeg | [C++ Serving](../examples/C++/PaddleSeg/unet_for_image_seg) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageSegmentation/unet.tar.gz) |

---
<a name="1.8"></a>

**八.关键点检测**

模型部署示例请参阅下表：
| 场景| 模型 | 类型 | 示例使用的框架 | 下载 |
| --- | --- | --- | --- | ---- |
| 关键点检测 |faster_rcnn_hrnetv2p_w18_1x | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/faster_rcnn_hrnetv2p_w18_1x) | [.tar.gz](https://paddle-serving.bj.bcebos.com/pddet_demo/faster_rcnn_hrnetv2p_w18_1x.tar.gz) |

---
<a name="1.9"></a>

**九.视频理解**

模型部署示例请参阅下表：
| 场景| 模型 | 类型 | 示例使用的框架 | 下载 |
| --- | --- | --- | --- | ---- |
| 视频理解 |PPTSN_K400 | PaddleVideo | [Pipeline Serving](../examples/Pipeline/PaddleVideo/PPTSN_K400) | [model](https://paddle-serving.bj.bcebos.com/model/PaddleVideo/PPTSN_K400.tar) |

---
<a name="2"></a>

## 模型示例库

Paddle Serving 代码库下模型部署示例请参考 [examples](../examples) 目录。更多 Paddle Serving 部署模型请参考 [wholechain](https://www.paddlepaddle.org.cn/wholechain)。

了解最新模型，请进入 Paddle 模型套件库：
  - [PaddleDetection](https://github.com/PaddlePaddle/PaddleDetection)
  - [PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR) 
  - [PaddleClas](https://github.com/PaddlePaddle/PaddleClas)
  - [PaddleNLP](https://github.com/PaddlePaddle/PaddleNLP)
  - [PaddleRec](https://github.com/PaddlePaddle/PaddleRec)
  - [PaddleSeg](https://github.com/PaddlePaddle/PaddleSeg)
  - [PaddleGAN](https://github.com/PaddlePaddle/PaddleGAN)
  - [PaddleVideo](https://github.com/PaddlePaddle/PaddleVideo)
