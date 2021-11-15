# hwvideoframe

Hwvideoframe is a CV preprocessing library based on cuda. The project uses GPU for image preprocessing operations. It speeds up the processing speed while increasing the utilization rate of the GPU.

## Preprocess API

Hwvideoframe provides a variety of data preprocessing methods for photo preprocess：

- class Image2Gpubuffer

  - `__call__(img)`
    - img（np.array）：Image data.

- class Gpubuffer2Image

  - `__call__(img)`
    - img（np.array）：Image data.

- class Div

  - `__init__(value)`
    - value（float）：Constant value to be divided.
  - `__call__(img)`
    - img（np.array）：Image data.

- class Sub

  - `__init__(subtractor)`
    - subtractor（list/float）：Three 32-bit floating point channel image subtract constant. When the input is a list type, length of list must be three.
  - `__call__(img)`
    - img（np.array）：Image data in (C,H,W) channels.

- class Normalize

  - `__init__(mean,std)`
    - mean（list）：Mean. Length of list must be three.
    - std（list）：Variance. Length of list must be three.
  - `__call__(img)`
    - img（np.array）：Image data in (C,H,W) channels.

- class CenterCrop

  - `__init__(size)`
    - size（int）：Crops the given Image at the center while the size must not bigger than any inputs' height and width.
  - `__call__(img)`
    - img（np.array）：Image data in (C,H,W) channels.

- class Resize

  - `__init__(size, max_size=2147483647, interpolation=None)`
    - size（list/int）：The expected image size, when the input is a list type, it needs to contain the expected length and width. When the input is int type, the short side will be set to the length of size, and the long side will be scaled proportionally.
  - `__call__(img)`
    - img（numpy array）：Image data in (C,H,W) channels.

## Quick start

[After compiling from code](../../../doc/Compile_EN.md)，this project will be stored in reader。

## How to Test

Test file：Serving/python/paddle_serving_app/reader/test_preprocess.py
