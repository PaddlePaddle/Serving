# Lod Introduction

(English|[简体中文](LOD_CN.md))

## Principle

Explanation of Tensor and Padding, Please refer to [paddlepaddle official website description](https://www.paddlepaddle.org.cn/documentation/docs/zh/1.8/beginners_guide/basic_concept/tensor.html).

LoD(Level-of-Detail) Tensor is an advanced feature of paddle and an extension of tensor. LoD Tensor improves training efficiency by sacrificing flexibility, reducing the impact of data expansion brought by padding.

**Notice：** For most users, there is no need to pay attention to the usage of LoD Tensor. Currently, serving only supports the usage of one-dimensional LOD.


## Use

**Prerequisite：** Your prediction model needs to support variable length tensor input.


Take the visual task as an example. In the visual task, we often need to process video and image. These elements are high-dimensional objects. 
Suppose that an existing Mini batch contains three videos, each video contains three frames, one frame and two frames respectively.
If each frame has the same size: 640x480, the mini batch can be expressed as:
```
3     1  2
口口口 口 口口
```
The size of the bottom tenor is (3 + 1 + 2) x640x480, and each 口 represents a 640x480 image.

Then, the shape of tensor is [6,640,480], lod=[0,3,4,6].

Where 0 is the starting value and 3-0 = 3; 4-3=1; 6-4 = 2, these three values just represent your variable length information. 

The last element 6 in LOD should be equal to the total length of the first dimension in shape.

The variable length information recorded in LOD and the first dimension information of shape in tensor should be aligned in the above way.
