# Model Zoo

(English|[简体中文](./Model_Zoo_CN.md)）

This page lists model archives that are pre-trained and pre-packaged, ready to be served for inference with PaddleServing. 
 To propose a model for inclusion, please submit [pull request](https://github.com/PaddlePaddle/Serving/pulls)

Special thanks to the [Padddle wholechain](https://www.paddlepaddle.org.cn/wholechain) and [PaddleHub](https://www.paddlepaddle.org.cn/hub) whose Model Zoo and Model Examples were used in generating these model archives


| Model | Type | Framework | Download |
| --- | --- | --- | ---- |
| pp_shitu | PaddleClas | [C++ Serving](../examples/C++/PaddleClas/pp_shitu) | [.tar.gz](https://paddle-imagenet-models-name.bj.bcebos.com/dygraph/rec/models/inference/serving/pp_shitu.tar.gz) |
| resnet_v2_50_imagenet | PaddleClas | [C++ Serving](../examples/C++/PaddleClas/resnet_v2_50)</br>[Pipeline Serving](../examples/Pipeline/PaddleClas/ResNet_V2_50) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageClassification/resnet_v2_50_imagenet.tar.gz) | Pipeline Serving, C++ Serving|
| mobilenet_v2_imagenet | PaddleClas | [C++ Serving](../examples/C++/PaddleClas/mobilenet) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageClassification/mobilenet_v2_imagenet.tar.gz) |
| resnet50_vd | PaddleClas | [C++ Serving](../examples/C++/PaddleClas/imagenet)</br>[Pipeline Serving](../examples/Pipeline/PaddleClas/ResNet50_vd) | [.tar.gz](https://paddle-serving.bj.bcebos.com/model/ResNet50_vd.tar) |
| ResNet50_vd_KL | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/ResNet50_vd_KL) | [.tar](https://paddle-serving.bj.bcebos.com/model/ResNet50_vd_KL.tar) |
| ResNet50_vd_FPGM | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/ResNet50_vd_FPGM) | [.tar](https://paddle-serving.bj.bcebos.com/model/ResNet50_vd_FPGM.tar) |
| ResNet50_vd_PACT | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/ResNet50_vd_PACT) | [.tar](https://paddle-serving.bj.bcebos.com/model/ResNet50_vd_PACT.tar) |
| ResNeXt101_vd_64x4d | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/ResNeXt101_vd_64x4d) | [.tar](https://paddle-serving.bj.bcebos.com/model/ResNeXt101_vd_64x4d.tar) |
| DarkNet53 | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/DarkNet53) | [.tar](https://paddle-serving.bj.bcebos.com/model/DarkNet53.tar) |
| MobileNetV1 | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/MobileNetV1) | [.tar](https://paddle-serving.bj.bcebos.com/model/MobileNetV1.tar) |
| MobileNetV2 | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/MobileNetV2) | [.tar](https://paddle-serving.bj.bcebos.com/model/MobileNetV2.tar) |
| MobileNetV3_large_x1_0 | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/MobileNetV3_large_x1_0) | [.tar](https://paddle-serving.bj.bcebos.com/model/MobileNetV3_large_x1_0.tar) |
| HRNet_W18_C | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/HRNet_W18_C) | [.tar](https://paddle-serving.bj.bcebos.com/model/HRNet_W18_C.tar) |
| ShuffleNetV2_x1_0 | PaddleClas | [Pipeline Serving](../examples/Pipeline/PaddleClas/ShuffleNetV2_x1_0) | [.tar](https://paddle-serving.bj.bcebos.com/model/ShuffleNetV2_x1_0.tar) |
| bert_chinese_L-12_H-768_A-12 | PaddleNLP | [C++ Serving](../examples/C++/PaddleNLP/bert)</br>[Pipeline Serving](../examples/Pipeline/PaddleNLP/bert) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SemanticModel/bert_chinese_L-12_H-768_A-12.tar.gz) |
| senta_bilstm | PaddleNLP | [C++ Serving](../examples/C++/PaddleNLP/senta) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SentimentAnalysis/senta_bilstm.tar.gz) |C++ Serving|
| lac | PaddleNLP | [C++ Serving](../examples/C++/PaddleNLP/lac) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/LexicalAnalysis/lac.tar.gz) |
| transformer | PaddleNLP | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleNLP/blob/develop/examples/machine_translation/transformer/deploy/serving/README.md) | [model](https://github.com/PaddlePaddle/PaddleNLP/tree/develop/examples/machine_translation/transformer) |
| criteo_ctr | PaddleRec | [C++ Serving](../examples/C++/PaddleRec/criteo_ctr) | [.tar.gz](https://paddle-serving.bj.bcebos.com/criteo_ctr_example/criteo_ctr_demo_model.tar.gz) |
| criteo_ctr_with_cube | PaddleRec | [C++ Serving](../examples/C++/PaddleRec/criteo_ctr_with_cube) | [.tar.gz](https://paddle-serving.bj.bcebos.com/unittest/ctr_cube_unittest.tar.gz) |
| wide&deep | PaddleRec | [C++ Serving](https://github.com/PaddlePaddle/PaddleRec/blob/release/2.1.0/doc/serving.md) | [model](https://github.com/PaddlePaddle/PaddleRec/blob/release/2.1.0/models/rank/wide_deep/README.md) |
| blazeface | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/blazeface) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ObjectDetection/blazeface.tar.gz) |C++ Serving|
| cascade_mask_rcnn_r50_vd_fpn_ssld_2x_coco | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/cascade_rcnn) | [.tar.gz](https://paddle-serving.bj.bcebos.com/pddet_demo/cascade_mask_rcnn_r50_vd_fpn_ssld_2x_coco_serving.tar.gz) |
| yolov4 | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/yolov4) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ObjectDetection/yolov4.tar.gz) |C++ Serving|
| faster_rcnn_hrnetv2p_w18_1x | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/faster_rcnn_hrnetv2p_w18_1x) | [.tar.gz](https://paddle-serving.bj.bcebos.com/pddet_demo/faster_rcnn_hrnetv2p_w18_1x.tar.gz) |
| fcos_dcn_r50_fpn_1x_coco | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/fcos_dcn_r50_fpn_1x_coco) | [.tar.gz](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/fcos_dcn_r50_fpn_1x_coco.tar) |
| ssd_vgg16_300_240e_voc | PaddleDetection |  [C++ Serving](../examples/C++/PaddleDetection/ssd_vgg16_300_240e_voc) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ssd_vgg16_300_240e_voc.tar) |
| yolov3_darknet53_270e_coco  | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/yolov3_darknet53_270e_coco)</br>[Pipeline Serving](../examples/Pipeline/PaddleDetection/yolov3) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/yolov3_darknet53_270e_coco.tar) |
| faster_rcnn_r50_fpn_1x_coco | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/faster_rcnn_r50_fpn_1x_coco)</br>[Pipeline Serving](../examples/Pipeline/PaddleDetection/faster_rcnn) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/faster_rcnn_r50_fpn_1x_coco.tar) |
| ppyolo_r50vd_dcn_1x_coco | PaddleDetection |  [C++ Serving](../examples/C++/PaddleDetection/ppyolo_r50vd_dcn_1x_coco) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_r50vd_dcn_1x_coco.tar) |
| ppyolo_mbv3_large_coco | PaddleDetection |  [Pipeline Serving](../examples/Pipeline/PaddleDetection/ppyolo_mbv3) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_mbv3_large_coco.tar) |
| ttfnet_darknet53_1x_coco | PaddleDetection | [C++ Serving](../examples/C++/PaddleDetection/ttfnet_darknet53_1x_coco) | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/ttfnet_darknet53_1x_coco.tar) |
| YOLOv3-DarkNet | PaddleDetection | [C++ Serving](https://github.com/PaddlePaddle/PaddleDetection/tree/release/2.3/deploy/serving) | [.pdparams](https://paddledet.bj.bcebos.com/models/yolov3_darknet53_270e_coco.pdparams)</br>[.yml](https://github.com/PaddlePaddle/PaddleDetection/blob/develop/configs/yolov3/yolov3_darknet53_270e_coco.yml) |
| ocr_rec | PaddleOCR | [C++ Serving](../examples/C++/PaddleOCR/ocr)</br>[Pipeline Serving](../examples/Pipeline/PaddleOCR/ocr) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/OCR/ocr_rec.tar.gz) |
| ocr_det | PaddleOCR | [C++ Serving](../examples/C++/PaddleOCR/ocr)</br>[Pipeline Serving](../examples/Pipeline/PaddleOCR/ocr) | [.tar.gz](https://paddle-serving.bj.bcebos.com/ocr/ocr_det.tar.gz) |
| ch_ppocr_mobile_v2.0_det | PaddleOCR | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/deploy/pdserving/README.md) | [model](https://paddleocr.bj.bcebos.com/dygraph_v2.0/ch/ch_ppocr_mobile_v2.0_det_infer.tar)</br>[.yml](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/configs/det/ch_ppocr_v2.0/ch_det_mv3_db_v2.0.yml) |
| ch_ppocr_server_v2.0_det | PaddleOCR | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/deploy/pdserving/README.md) | [model](https://paddleocr.bj.bcebos.com/dygraph_v2.0/ch/ch_ppocr_server_v2.0_det_infer.tar)</br>[.yml](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/configs/det/ch_ppocr_v2.0/ch_det_res18_db_v2.0.yml) |
| ch_ppocr_mobile_v2.0_rec | PaddleOCR | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/deploy/pdserving/README.md) | [model](https://paddleocr.bj.bcebos.com/dygraph_v2.0/ch/ch_ppocr_mobile_v2.0_rec_infer.tar)</br>[.yml](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/configs/rec/ch_ppocr_v2.0/rec_chinese_lite_train_v2.0.yml) |
| ch_ppocr_server_v2.0_rec | PaddleOCR | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/deploy/pdserving/README.md) | [model](https://paddleocr.bj.bcebos.com/dygraph_v2.0/ch/ch_ppocr_server_v2.0_rec_infer.tar)</br>[.yml](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/configs/rec/ch_ppocr_v2.0/rec_chinese_common_train_v2.0.yml) |
| ch_ppocr_mobile_v2.0 | PaddleOCR | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/deploy/pdserving/README.md) | [model](https://github.com/PaddlePaddle/PaddleOCR) |
| ch_ppocr_server_v2.0 | PaddleOCR | [Pipeline Serving](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/deploy/pdserving/README.md) | [model](https://github.com/PaddlePaddle/PaddleOCR) |
| deeplabv3 | PaddleSeg | [C++ Serving](../examples/C++/PaddleSeg/deeplabv3) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageSegmentation/deeplabv3.tar.gz) |
| unet | PaddleSeg | [C++ Serving](../examples/C++/PaddleSeg/unet_for_image_seg) | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageSegmentation/unet.tar.gz) |
| PPTSN_K400 | PaddleVideo | [Pipeline Serving](../examples/Pipeline/PaddleVideo/PPTSN_K400) | [model](https://paddle-serving.bj.bcebos.com/model/PaddleVideo/PPTSN_K400.tar) |

- Refer [example](../examples) for more details on above models.

- Refer [wholechain](https://www.paddlepaddle.org.cn/wholechain) for more pre-trained models supported by PaddleServing

- Latest models refer
  - [PaddleDetection](https://github.com/PaddlePaddle/PaddleDetection)
  - [PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR) 
  - [PaddleClas](https://github.com/PaddlePaddle/PaddleClas)
  - [PaddleNLP](https://github.com/PaddlePaddle/PaddleNLP)
  - [PaddleRec](https://github.com/PaddlePaddle/PaddleRec)
  - [PaddleSeg](https://github.com/PaddlePaddle/PaddleSeg)
  - [PaddleGAN](https://github.com/PaddlePaddle/PaddleGAN)
  - [PaddleVideo](https://github.com/PaddlePaddle/PaddleVideo)
