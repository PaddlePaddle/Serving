from paddle_serving_client import Client
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import benchmark_args
import time
import paddle
import sys
import requests

args = benchmark_args()

def single_func(idx, resource):
    if args.request == "rpc":
        client = Client()
        client.load_client_config(args.model)
        client.connect([args.endpoint])
        train_reader = paddle.batch(paddle.reader.shuffle(
            paddle.dataset.uci_housing.train(), buf_size=500), batch_size=1)
        start = time.time()
        for data in train_reader():
            fetch_map = client.predict(feed={"x": data[0][0]}, fetch=["price"])
        end = time.time()
        return [[end - start]]
    elif args.request == "http":
        train_reader = paddle.batch(paddle.reader.shuffle(
            paddle.dataset.uci_housing.train(), buf_size=500), batch_size=1)
        start = time.time()
        for data in train_reader():
            r = requests.post('http://{}/uci/prediction'.format(args.endpoint),
                              data = {"x": data[0]})
        end = time.time()
        return [[end - start]]

multi_thread_runner = MultiThreadRunner()
result = multi_thread_runner.run(single_func, args.thread, {})
print(result)
