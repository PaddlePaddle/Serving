# hwvideoframe
hwvideoframe是一个基于cuda的图片预处理库。项目利用GPU进行图片预处理操作，加快处理速度的同时，提高GPU的使用率。

## 项目结构
hwvideoframe通过cpp的
hwvideoframe目前提供以下图片预处理功能：
* Image2Gpubuffer
* Gpubuffer2Image
* RGB2BGR
* BGR2RGB
* Div
* Sub
* Normalize
* CenterCrop
* Resize
* ResizeByFactor

## 快速开始
按照Paddle Serving文档编译，编译结果在reader中。

## 测试
测试文件：Serving/python/paddle_serving_app/reader/test_preprocess.py
