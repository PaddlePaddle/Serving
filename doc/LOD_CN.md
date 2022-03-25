# Lod字段说明

(简体中文|[English](LOD_EN.md))

## 概念

Tensor以及Padding的概念说明请参考[PaddlePaddle官网说明](https://www.paddlepaddle.org.cn/documentation/docs/zh/1.8/beginners_guide/basic_concept/tensor.html)

LoD(Level-of-Detail) Tensor是Paddle的高级特性，是对Tensor的一种扩充。LoDTensor通过牺牲灵活性来提升训练的效率,减少padding带来的数据扩充的影响。

**注：** 对于大部分用户来说，无需关注LoDTensor的用法，目前Serving中仅支持一维Lod的用法。


## 使用

**前提：** 首先您的预测模型需要支持变长Tensor的输入。


以视觉任务为例，在视觉任务中，时常需要处理视频和图像这些元素是高维的对象，假设现存的一个mini-batch包含3个视频，分别有3个，1个和2个帧。
每个帧都具有相同大小：640x480，则这个mini-batch可以被表示为：
```
3     1  2
口口口 口 口口
```
最底层tensor大小为（3+1+2）x640x480，每一个 口 表示一个640x480的图像。

那么此时，Tensor的shape为[6,640,480],lod=[0,3,4,6].

其中0为起始值，3-0=3;4-3=1;6-4=2，这三个值正好表示您的变长信息，lod中的最后一个元素6，应等于shape中第一维度的总长度。

lod中记录的变长信息与Tensor中shape的第一维度的信息应按照上述方式对齐。
