from paddle_serving_client import MultiLangClient as Client
from paddle_serving_app.reader import *
import sys
import numpy as np

preprocess = Sequential([
    File2Image(),
    Normalize([104, 117, 123], [127.502231, 127.502231, 127.502231], False) 
])

postprocess =  BlazeFacePostprocess("label_list.txt", "output")
client = Client()

client.connect(['127.0.0.1:9393'])

im_0 = preprocess(sys.argv[1])
tmp = Transpose((2,0,1))
im = tmp(im_0)
fetch_map = client.predict(
    feed={
        "image": im
    },
    fetch=["detection_output_0.tmp_0"], batch=True)
print(fetch_map)
fetch_map["image"] = sys.argv[1]
fetch_map["im_shape"] = im_0.shape
postprocess(fetch_map)
