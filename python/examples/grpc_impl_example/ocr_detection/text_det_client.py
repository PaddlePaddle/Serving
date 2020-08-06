import os
from paddle_serving_client import MultiLangClient as Client
from paddle_serving_app.reader import Sequential, File2Image, ResizeByFactor
from paddle_serving_app.reader import Div, Normalize, Transpose
from paddle_serving_app.reader import DBPostProcess, FilterBoxes

client = Client()
client.connect(["127.0.0.1:9393"])

read_image_file = File2Image()
preprocess = Sequential(
    [ResizeByFactor(32, 960), Div(255),
     Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225]),
     Transpose((2, 0, 1))])
post_func = DBPostProcess(
    {"thresh": 0.3, "box_thresh": 0.5,
     "max_candidates": 1000, "unclip_ratio": 1.5,
     "min_size": 3})
filter_func = FilterBoxes(10, 10)
name = "7.jpg"
img = read_image_file(name)
ori_h, ori_w, _ = img.shape
img = preprocess(img)
new_h, new_w, _ = img.shape
ratio_list = [float(new_h) / ori_h, float(new_w) / ori_w]
outputs = client.predict(feed={"image": img}, fetch=["concat_1.tmp_0"], batch=True)
dt_boxes_list = post_func(outputs["concat_1.tmp_0"], [ratio_list])
dt_boxes = filter_func(dt_boxes_list[0], [ori_h, ori_w])
