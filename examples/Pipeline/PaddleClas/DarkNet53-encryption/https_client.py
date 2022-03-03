import numpy as np
import requests
import json
import cv2
import base64
import os

def cv2_to_base64(image):
    return base64.b64encode(image).decode('utf8')

if __name__ == "__main__":
    url = "https://10.21.8.132:8443/image-clas/imagenet/prediction"
    with open(os.path.join(".", "daisy.jpg"), 'rb') as file:
        image_data1 = file.read()
    image = cv2_to_base64(image_data1)
    headers = {"Content-Type":"application/json", "apikey":"BlfvO08Z9mQpFjcMagl2dxOIA8h2UVdp", "X-INSTANCE-ID" : "kong_ins10"}
    data = {"key": ["image"], "value": [image]}
    for i in range(1):
        r = requests.post(url=url, headers=headers, data=json.dumps(data),verify=False)
        print(r.json())
