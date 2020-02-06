import sys
import paddle
import paddle.fluid as fluid

train_reader = paddle.batch(paddle.reader.shuffle(
    paddle.dataset.uci_housing.train(), buf_size=500), batch_size=16)

test_reader = paddle.batch(paddle.reader.shuffle(
    paddle.dataset.uci_housing.test(), buf_size=500), batch_size=16)

x = fluid.data(name='x', shape=[None, 13], dtype='float32')
y = fluid.data(name='y', shape=[None, 1], dtype='float32')

y_predict = fluid.layers.fc(input=x, size=1, act=None)
cost = fluid.layers.square_error_cost(input=y_predict, label=y)
avg_loss = fluid.layers.mean(cost)
sgd_optimizer = fluid.optimizer.SGD(learning_rate=0.01)
sgd_optimizer.minimize(avg_loss)

place = fluid.CPUPlace()
feeder = fluid.DataFeeder(place=place, feed_list=[x, y])
exe = fluid.Executor(place)
exe.run(fluid.default_startup_program())

import paddle_serving_client.io as serving_io

for pass_id in range(30):
    for data_train in train_reader():
        avg_loss_value, = exe.run(
            fluid.default_main_program(),
            feed=feeder.feed(data_train),
            fetch_list=[avg_loss])

serving_io.save_model(
    "serving_server_model", "serving_client_conf",
    {"x": x}, {"y": y_predict}, fluid.default_main_program())
