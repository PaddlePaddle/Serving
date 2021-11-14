#   Copyright (c) 2018 PaddlePaddle Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# pylint: disable=doc-string-missing
import os
import sys
import paddle
import logging
import paddle.fluid as fluid

logging.basicConfig(format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger("fluid")
logger.setLevel(logging.INFO)
paddle.enable_static()


def load_vocab(filename):
    vocab = {}
    with open(filename) as f:
        wid = 0
        for line in f:
            vocab[line.strip()] = wid
            wid += 1
    vocab["<unk>"] = len(vocab)
    return vocab


if __name__ == "__main__":
    from nets import lstm_net
    model_name = "imdb_lstm"
    vocab = load_vocab('imdb.vocab')
    dict_dim = len(vocab)

    data = fluid.layers.data(
        name="words", shape=[1], dtype="int64", lod_level=1)
    label = fluid.layers.data(name="label", shape=[1], dtype="int64")

    dataset = fluid.DatasetFactory().create_dataset()
    filelist = ["train_data/%s" % x for x in os.listdir("train_data")]
    dataset.set_use_var([data, label])
    pipe_command = "python imdb_reader.py"
    dataset.set_pipe_command(pipe_command)
    dataset.set_batch_size(128)
    dataset.set_filelist(filelist)
    dataset.set_thread(10)
    avg_cost, acc, prediction = lstm_net(data, label, dict_dim)
    optimizer = fluid.optimizer.SGD(learning_rate=0.01)
    optimizer.minimize(avg_cost)

    exe = fluid.Executor(fluid.CPUPlace())
    exe.run(fluid.default_startup_program())
    epochs = 6

    import paddle_serving_client.io as serving_io

    for i in range(epochs):
        exe.train_from_dataset(
            program=fluid.default_main_program(), dataset=dataset, debug=False)
        logger.info("TRAIN --> pass: {}".format(i))
        if i == 5:
            serving_io.save_model("{}_model".format(model_name),
                                  "{}_client_conf".format(model_name),
                                  {"words": data}, {"prediction": prediction},
                                  fluid.default_main_program())
