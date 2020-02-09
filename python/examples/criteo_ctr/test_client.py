from paddle_serving_client import Client
import paddle
import sys
import os
import criteo_reader as criteo

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9292"])

batch = 1
buf_size = 100
dataset = criteo.CriteoDataset()
dataset.setup(1000001)
test_filelists = ["{}/part-%d".format(sys.argv[2]) % x
                 for x in range(len(os.listdir(sys.argv[2])))]
reader = dataset.infer_reader(test_filelists[len(test_filelists)-40:], batch, buf_size)

for data in reader():
    feed_dict = {}
    feed_dict["dense_0"] = data[0][0]
    for i in range(1, 27):
        feed_dict["sparse_{}".format(i - 1)] = data[0][i]
    feed_dict["label"] = data[0][-1]
    fetch_map = client.predict(feed=feed_dict, fetch=["prob"])
    print("{} {}".format(fetch_map["prob"][0], data[0][-1][0]))
    
