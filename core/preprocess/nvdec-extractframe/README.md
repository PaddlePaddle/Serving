# hw-frame-extract
hw-frame-extract is a CV preprocessing library based on cuda. The project uses GPU for extracting frame from video. It speeds up the processing speed while increasing the utilization rate of the GPU.

## Preprocess API
Hwvideoframe provides a variety of cutting frame methods for different kind of video：

- class HwExtractFrameJpeg

  - `init_handler(gpu_index)`
    - gpu_index (int)：Index of gpu.
  - `extract_frame(file_path, n)`
    - file_path (string): Video path.
    - n (int): Cut n frames in one second.

- class HwExtractFrameBGRARaw

  - `init_handler(gpu_index)`
    - gpu_index (int)：Index of gpu.
  - `extract_frame(file_path, n)`
    - file_path (string): Video path.
    - n (int): Cut n frames in one second.

- class HwFrameResult

  - `height()`
    - Return height of frame.
  - `width()`
    - Return width of frame.
  - `len()`
    - Return len of frame.
  - `free_memory()`
    - Free memory of frames.

## Quick start
[After compiling from code](https://github.com/PaddlePaddle/Serving/blob/develop/doc/COMPILE.md)，this project will be stored in reader。

## How to Test
Test file：Serving/python/paddle_serving_app/reader/test_preprocess.py
