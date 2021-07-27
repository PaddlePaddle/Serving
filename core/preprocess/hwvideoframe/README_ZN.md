# hwvideoframe

hwvideoframe是一个基于cuda的图片预处理库。项目利用GPU进行图片预处理操作，在加快处理速度的同时，提高GPU的使用率。

## 项目结构

hwvideoframe目前提供以下图片预处理功能：

- class Image2Gpubuffer

  - `__call__(img)`
    - img（np.array）：输入图像。

- class Gpubuffer2Image

  - `__call__(img)`
    - img（np.array）：输入图像。

- class Div

  - `__init__(value)`
    - value（float）：根据固定值切割图像。
  - `__call__(img)`
    - img（np.array）：输入图像。

- class Sub

  - `__init__(subtractor)`
    - subtractor（list/float）：list的长度必须为3。
  - `__call__(img)`
    - img（np.array）：（C,H,W）排列的图像数据。

- class Normalize

  - `__init__(mean,std)`
    - mean（list）：均值。 list长度必须为3。
    - std（list）：方差。 list长度必须为3。
  - `__call__(img)`
    - img（np.array）：（C,H,W）排列的图像数据。

- class CenterCrop

  - `__init__(size)`
    - size（int）：预期的裁剪后的大小，list类型时需要包含预期的长和宽，int类型时会返回边长为size的正方形图片。size不能大于原始图片大小。
  - `__call__(img)`
    - img（np.array）：（C,H,W）排列的图像数据

- class Resize

  - `__init__(size, max_size=2147483647, interpolation=None)`
    - size（list/int）：预期的图像大小，短边会设置为size的长度，长边按比例缩放.
  - `__call__(img)`
    - img（numpy array）：（C,H,W）排列的图像数据

## 快速开始

按照Paddle Serving文档编译，编译结果在reader中。

## 测试

测试文件：Serving/python/paddle_serving_app/reader/test_preprocess.py
