# Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

from __future__ import print_function

from args import parse_args
import os
import paddle.fluid as fluid
import paddle
import sys
from network_conf import dnn_model

dense_feature_dim = 13

paddle.enable_static()


def train():
    args = parse_args()
    sparse_only = args.sparse_only
    if not os.path.isdir(args.model_output_dir):
        os.mkdir(args.model_output_dir)
    dense_input = fluid.layers.data(
        name="dense_input", shape=[dense_feature_dim], dtype='float32')
    sparse_input_ids = [
        fluid.layers.data(
            name="C" + str(i), shape=[1], lod_level=1, dtype="int64")
        for i in range(1, 27)
    ]
    label = fluid.layers.data(name='label', shape=[1], dtype='int64')

    #nn_input = None if sparse_only else dense_input
    nn_input = dense_input
    predict_y, loss, auc_var, batch_auc_var, infer_vars = dnn_model(
        nn_input, sparse_input_ids, label, args.embedding_size,
        args.sparse_feature_dim)

    optimizer = fluid.optimizer.SGD(learning_rate=1e-4)
    optimizer.minimize(loss)

    exe = fluid.Executor(fluid.CPUPlace())
    exe.run(fluid.default_startup_program())
    dataset = fluid.DatasetFactory().create_dataset("InMemoryDataset")
    dataset.set_use_var([dense_input] + sparse_input_ids + [label])

    python_executable = "python3.6"
    pipe_command = "{} criteo_reader.py {}".format(python_executable,
                                                   args.sparse_feature_dim)

    dataset.set_pipe_command(pipe_command)
    dataset.set_batch_size(128)
    thread_num = 10
    dataset.set_thread(thread_num)

    whole_filelist = [
        "raw_data/part-%d" % x for x in range(len(os.listdir("raw_data")))
    ]

    print(whole_filelist)
    dataset.set_filelist(whole_filelist[:100])
    dataset.load_into_memory()
    fluid.layers.Print(auc_var)
    epochs = 1
    for i in range(epochs):
        exe.train_from_dataset(
            program=fluid.default_main_program(), dataset=dataset, debug=True)
        print("epoch {} finished".format(i))

    import paddle_serving_client.io as server_io
    feed_var_dict = {}
    feed_var_dict['dense_input'] = dense_input
    for i, sparse in enumerate(sparse_input_ids):
        feed_var_dict["embedding_{}.tmp_0".format(i)] = sparse
    fetch_var_dict = {"prob": predict_y}

    feed_kv_dict = {}
    feed_kv_dict['dense_input'] = dense_input
    for i, emb in enumerate(infer_vars):
        feed_kv_dict["embedding_{}.tmp_0".format(i)] = emb
    fetch_var_dict = {"prob": predict_y}

    server_io.save_model("ctr_serving_model", "ctr_client_conf", feed_var_dict,
                         fetch_var_dict, fluid.default_main_program())

    server_io.save_model("ctr_serving_model_kv", "ctr_client_conf_kv",
                         feed_kv_dict, fetch_var_dict,
                         fluid.default_main_program())


if __name__ == '__main__':
    train()
