from paddle_serving_client import Client
from paddle_serving_app.reader import *
import sys
import numpy as np

preprocess = Sequential([
    File2Image(), BGR2RGB(), Div(255.0),
    Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225], False),
    Resize(640, 640), Transpose((2, 0, 1))
])

postprocess = RCNNPostprocess("label_list.txt", "output")
client = Client()

client.load_client_config("serving_client/serving_client_conf.prototxt")
client.connect(['127.0.0.1:9494'])

im = preprocess(sys.argv[1])
fetch_map = client.predict(
    feed={
        "image": im,
        "im_info": np.array(list(im.shape[1:]) + [1.0]),
        "im_shape": np.array(list(im.shape[1:]) + [1.0])
    },
    fetch=["multiclass_nms_0.tmp_0"],
    batch=False)
print(fetch_map)
