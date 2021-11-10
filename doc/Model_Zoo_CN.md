# Model Zoo

本页面展示了Paddle Serving目前支持的预训练模型以及下载链接
若您想为Paddle Serving提供新的模型，可通过[pull requese](https://github.com/PaddlePaddle/Serving/pulls)提交PR

*特别感谢[PadddlePaddle 全链条](https://www.paddlepaddle.org.cn/wholechain)以及[PaddleHub](https://www.paddlepaddle.org.cn/hub)为Paddle Serving提供的部分预训练模型*


| Model | Type | Dataset | Size | Download | Sample Input| Model mode |
| --- | --- | --- | --- | --- | --- | --- |
| ResNet V2 50 | PaddleClas | ImageNet | 90.78 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageClassification/resnet_v2_50_imagenet.tar.gz) | [daisy.jpg](../examples/PaddleClas/resnet_v2_50/daisy.jpg) |Eager|
| MobileNet_v2 | PaddleClas | ImageNet | 8.06 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageClassification/mobilenet_v2_imagenet.tar.gz) | [daisy.jpg](../examples/PaddleClas/mobilenet/daisy.jpg) |Eager|
| Bert | PaddleNLP | zhwiki | 361.96 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SemanticModel/bert_chinese_L-12_H-768_A-12.tar.gz) | [data-c.txt](../examples/PaddleNLP/data-c.txt) |Eager|
| Senta | PaddleNLP | Baidu | 578.37 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SentimentAnalysis/senta_bilstm.tar.gz) |  |Eager|
| Lac | PaddleNLP | Baidu | 11.38 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/LexicalAnalysis/lac.tar.gz) |  |Eager|
| Criteo Ctr | PaddleRec |  | 36.72 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/criteo_ctr_example/criteo_ctr_demo_model.tar.gz) | [ctr_data.tar.gz](https://paddle-serving.bj.bcebos.com/data/ctr_prediction/ctr_data.tar.gz) |Eager|
| Criteo Ctr With Cube | PaddleRec |  | 41.68 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/unittest/ctr_cube_unittest.tar.gz) | [ctr_data.tar.gz](https://paddle-serving.bj.bcebos.com/data/ctr_prediction/ctr_data.tar.gz) |Eager|
| BlazeFace | PaddleDetection | WIDER-FACE | 1.40 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ObjectDetection/blazeface.tar.gz) | [test.jpg](../examples/PaddleDetection/blazeface) |Eager|
| Cascade RCNN | PaddleDetection | ImageNet | 255.58 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/pddet_demo/cascade_mask_rcnn_r50_vd_fpn_ssld_2x_coco_serving.tar.gz) | [000000570688.jpg](../examples/PaddleDetection/cascade_rcnn/000000570688.jpg) |Eager|
| Yolov4 | PaddleDetection | ImageNet | 228.38 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ObjectDetection/yolov4.tar.gz) | [000000570688.jpg](../examples/PaddleDetection/yolov4/000000570688.jpg) |Eager|
| Faster RCNN HRNetv2p w18 1x | PaddleDetection | ImageNet | 97.57 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/pddet_demo/faster_rcnn_hrnetv2p_w18_1x.tar.gz) | [000000570688.jpg](../examples/PaddleDetection/faster_rcnn_hrnetv2p_w18_1x/000000570688.jpg) |Eager|
| Fcos DCN R50 FPN 1x coco | PaddleDetection | ImageNet | 119.55 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/fcos_dcn_r50_fpn_1x_coco.tar) | [000000570688.jpg](../examples/PaddleDetection/fcos_dcn_r50_fpn_1x_coco/000000570688.jpg) |Eager|
| SSD VGG16 300 240e VOC | PaddleDetection | ImageNet | 93.16 MB | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ssd_vgg16_300_240e_voc.tar) | [000000014439.jpg](../examples/PaddleDetection/ssd_vgg16_300_240e_voc/000000014439.jpg) |Torchscripted |
| Yolov3 DarkNet53 270e coco | PaddleDetection | ImageNet | 219.71 MB | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/yolov3_darknet53_270e_coco.tar) | [000000570688.jpg](../examples/PaddleDetection/yolov3_darknet53_270e_coco/000000570688.jpg) |Torchscripted |
| Faster RCNN r50 Fpn 1x coco | PaddleDetection | ImageNet | 148.12 MB | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/faster_rcnn_r50_fpn_1x_coco.tar) | [000000570688.jpg](../examples/PaddleDetection/faster_rcnn_r50_fpn_1x_coco/000000570688.jpg) |Torchscripted |
| PPYolo r50vd DCN 1x coco | PaddleDetection | ImageNet | 167.03 MB | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_r50vd_dcn_1x_coco.tar) | [000000570688.jpg](../examples/PaddleDetection/ppyolo_r50vd_dcn_1x_coco/000000570688.jpg) |Torchscripted |
| TTFNet DarkNet53 1x coco | PaddleDetection | ImageNet | 162.19 MB | [.tar](https://paddle-serving.bj.bcebos.com/pddet_demo/ttfnet_darknet53_1x_coco.tar) | [000000570688.jpg](../examples/PaddleDetection/ttfnet_darknet53_1x_coco/000000570688.jpg) |Torchscripted |
| OCR REC | PaddleOCR |  | 3.80 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/OCR/ocr_rec.tar.gz) | [test_imgs.tar](https://paddle-serving.bj.bcebos.com/ocr/test_imgs.tar) |Torchscripted |
| Deeplabv3/ | PaddleSeg | Baidu | 146.97 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageSegmentation/deeplabv3.tar.gz) | [N0060.jpg](../examples/PaddleSeg/deeplabv3/N0060.jpg) |Torchscripted |
| UNet For Image Seg | PaddleSeg |  | 47.69 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageSegmentation/unet.tar.gz) | [N0060.jpg](../examples/PaddleSeg/unet_for_image_seg/N0060.jpg) |Torchscripted |

Refer [example](../examples) for more details on above models.
