# Model Zoo

本页面展示了Paddle Serving目前支持的预训练模型以及下载链接
若您想为Paddle Serving提供新的模型，可通过[pull requese](https://github.com/PaddlePaddle/Serving/pulls)提交PR

*特别感谢[PadddlePaddle 全链条](https://www.paddlepaddle.org.cn/wholechain)以及[PaddleHub](https://www.paddlepaddle.org.cn/hub)为Paddle Serving提供的部分预训练模型*


| Model | Type | Dataset | Size | Download | Sample Input| Model mode |
| --- | --- | --- | --- | --- | --- | --- |
| ResNet_V2_50 | PaddleClas | ImageNet | 90.78 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageClassification/resnet_v2_50_imagenet.tar.gz) | [daisy.jpg](../examples/PaddleClas/resnet_v2_50/daisy.jpg) |Eager|
| MobileNet_v2 | PaddleClas | ImageNet | 8.06 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageClassification/mobilenet_v2_imagenet.tar.gz) | [daisy.jpg](../examples/PaddleClas/mobilenet/daisy.jpg) |Eager|
| Bert | PaddleNLP | zhwiki | 361.96 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SemanticModel/bert_chinese_L-12_H-768_A-12.tar.gz) | [data-c.txt](../examples/PaddleNLP/data-c.txt) |Eager|
| Senta | PaddleNLP | Baidu | 578.37 MB | [.tar.gz](https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SentimentAnalysis/senta_bilstm.tar.gz) |  |Eager|
| Squeezenet 1_1 | Image Classification | ImageNet | 4.4 MB | [.mar](https://torchserve.pytorch.org/mar_files/squeezenet1_1.mar) | [kitten.jpg](../examples/image_classifier/kitten.jpg) |Eager|
| MNIST digit classifier | Image Classification | MNIST | 4.3 MB | [.mar](https://torchserve.pytorch.org/mar_files/mnist_v2.mar) | [0.png](../examples/image_classifier/mnist/test_data/0.png) |Eager|
| Resnet 152 |Image Classification | ImageNet | 214 MB | [.mar](https://torchserve.pytorch.org/mar_files/resnet-152-batch_v2.mar) | [kitten.jpg](../examples/image_classifier/kitten.jpg) |Eager|
| Faster RCNN | Object Detection | COCO | 148 MB | [.mar](https://torchserve.pytorch.org/mar_files/fastrcnn.mar) | [persons.jpg](../examples/object_detector/persons.jpg) |Eager|
| MASK RCNN | Object Detection | COCO | 158 MB | [.mar](https://torchserve.pytorch.org/mar_files/maskrcnn.mar) | [persons.jpg](../examples/object_detector/persons.jpg) |Eager|
| Text classifier | Text Classification | AG_NEWS | 169 MB | [.mar](https://torchserve.pytorch.org/mar_files/my_text_classifier_v4.mar) | [sample_text.txt](../examples/text_classification/sample_text.txt) |Eager|
| FCN ResNet 101 | Image Segmentation | COCO | 193 MB | [.mar](https://torchserve.pytorch.org/mar_files/fcn_resnet_101.mar) | [persons.jpg](../examples/image_segmenter/persons.jpg) |Eager|
| DeepLabV3 ResNet 101 | Image Segmentation | COCO | 217 MB | [.mar](https://torchserve.pytorch.org/mar_files/deeplabv3_resnet_101_eager.mar) | [persons.jpg](https://github.com/pytorch/serve/blob/master/examples/image_segmenter/persons.jpg) |Eager|
| AlexNet Scripted | Image Classification | ImageNet | 216 MB | [.mar](https://torchserve.pytorch.org/mar_files/alexnet_scripted.mar) | [kitten.jpg](../examples/image_classifier/kitten.jpg) |Torchscripted |
| Densenet161 Scripted| Image Classification | ImageNet | 105 MB | [.mar](https://torchserve.pytorch.org/mar_files/densenet161_scripted.mar) | [kitten.jpg](../examples/image_classifier/kitten.jpg) |Torchscripted |
| Resnet18 Scripted| Image Classification | ImageNet | 42 MB | [.mar](https://torchserve.pytorch.org/mar_files/resnet-18_scripted.mar) | [kitten.jpg](../examples/image_classifier/kitten.jpg) |Torchscripted |
| VGG16 Scripted| Image Classification | ImageNet | 489 MB | [.mar](https://torchserve.pytorch.org/mar_files/vgg16_scripted.mar) | [kitten.jpg](../examples/image_classifier/kitten.jpg) |Torchscripted |
| Squeezenet 1_1 Scripted | Image Classification | ImageNet | 4.4 MB | [.mar](https://torchserve.pytorch.org/mar_files/squeezenet1_1_scripted.mar) | [kitten.jpg](../examples/image_classifier/kitten.jpg) |Torchscripted |
| MNIST digit classifier Scripted | Image Classification | MNIST | 4.3 MB | [.mar](https://torchserve.pytorch.org/mar_files/mnist_scripted_v2.mar) | [0.png](../examples/image_classifier/mnist/test_data/0.png) |Torchscripted |
| Resnet 152 Scripted |Image Classification | ImageNet | 215 MB | [.mar](https://torchserve.pytorch.org/mar_files/resnet-152-scripted_v2.mar) | [kitten.jpg](../examples/image_classifier/kitten.jpg) |Torchscripted |
| Text classifier Scripted | Text Classification | AG_NEWS | 169 MB | [.mar](https://torchserve.pytorch.org/mar_files/my_text_classifier_scripted_v3.mar) | [sample_text.txt](../examples/text_classification/sample_text.txt) |Torchscripted |
| FCN ResNet 101 Scripted | Image Segmentation | COCO | 193 MB | [.mar](https://torchserve.pytorch.org/mar_files/fcn_resnet_101_scripted.mar) | [persons.jpg](../examples/image_segmenter/persons.jpg) |Torchscripted |
| DeepLabV3 ResNet 101 Scripted | Image Segmentation | COCO | 217 MB | [.mar](https://torchserve.pytorch.org/mar_files/deeplabv3_resnet_101_scripted.mar) | [persons.jpg](https://github.com/pytorch/serve/blob/master/examples/image_segmenter/persons.jpg) |Torchscripted |

Refer [example](../examples) for more details on above models.
