from paddle_serving_client import Client
import sys
from process import DetectFace

DetectFace = DetectFace("output")
client = Client()
# load client prototxt
client.load_client_config(sys.argv[1])
client.connect(['127.0.0.1:9494'])
image, _, _ = DetectFace.preprocess(sys.argv[2])
fetch_map = client.predict(
    feed={"image": image}, fetch=["save_infer_model/scale_0"])
fetch_map["image"] = sys.argv[2]
fetch_map["im_shape"] = image.shape
DetectFace.postprocess(fetch_map)
