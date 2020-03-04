from paddle_serving_client import Client
import paddle
import sys

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9393"])

test_reader = paddle.batch(paddle.reader.shuffle(
    paddle.dataset.uci_housing.test(), buf_size=500), batch_size=1)

for data in test_reader():
    fetch_map = client.predict(feed={"x": data[0][0]}, fetch=["price"])
    print("{} {}".format(fetch_map["price"][0], data[0][1][0]))
    
