from __future__ import print_function

import argparse
import logging
import os
import time
import math
import reader
import google.protobuf.text_format as text_format

import numpy as np
import six

import paddle
import paddle.fluid as fluid
import paddle.fluid.proto.framework_pb2 as framework_pb2
import paddle.fluid.core as core

from multiprocessing import cpu_count

# disable gpu training for this example
os.environ["CUDA_VISIBLE_DEVICES"] = ""

logging.basicConfig(
    format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger("fluid")
logger.setLevel(logging.INFO)

dense_feature_dim = 13

def parse_args():
    parser = argparse.ArgumentParser(description="PaddlePaddle CTR example")
    parser.add_argument(
        '--train_data_path',
        type=str,
        default='./data/raw/train.txt',
        help="The path of training dataset")
    parser.add_argument(
        '--batch_size',
        type=int,
        default=1000,
        help="The size of mini-batch (default:1000)")
    parser.add_argument(
        '--embedding_size',
        type=int,
        default=10,
        help="The size for embedding layer (default:10)")
    parser.add_argument(
        '--sparse_feature_dim',
        type=int,
        default=1000001,
        help='sparse feature hashing space for index processing')
    parser.add_argument(
        '--model_output_dir',
        type=str,
        default='models',
        help='The path for model to store (default: models)')
    return parser.parse_args()

def ctr_dnn_model(embedding_size, sparse_feature_dim, use_py_reader=True):

    def embedding_layer(input):
        emb = fluid.layers.embedding(
            input=input,
            is_sparse=True,
            # you need to patch https://github.com/PaddlePaddle/Paddle/pull/14190
            # if you want to set is_distributed to True
            is_distributed=False,
            size=[sparse_feature_dim, embedding_size],
            param_attr=fluid.ParamAttr(name="SparseFeatFactors",
                                       initializer=fluid.initializer.Uniform()))
        seq = fluid.layers.sequence_pool(input=emb, pool_type='average')
        return emb, seq

    dense_input = fluid.layers.data(
        name="dense_input", shape=[dense_feature_dim], dtype='float32')

    sparse_input_ids = [
        fluid.layers.data(name="C" + str(i), shape=[1], lod_level=1, dtype='int64')
        for i in range(1, 27)]

    label = fluid.layers.data(name='label', shape=[1], dtype='int64')

    words = [dense_input] + sparse_input_ids + [label]

    sparse_embed_and_seq = list(map(embedding_layer, words[1:-1]))
    emb_list = [x[0] for x in sparse_embed_and_seq]
    sparse_embed_seq = [x[1] for x in sparse_embed_and_seq]

    concated = fluid.layers.concat(sparse_embed_seq + words[0:1], axis=1)

    train_feed_vars = words
    inference_feed_vars = emb_list + words[0:1]

    fc1 = fluid.layers.fc(input=concated, size=400, act='relu',
                          param_attr=fluid.ParamAttr(initializer=fluid.initializer.Normal(
                              scale=1 / math.sqrt(concated.shape[1]))))
    fc2 = fluid.layers.fc(input=fc1, size=400, act='relu',
                          param_attr=fluid.ParamAttr(initializer=fluid.initializer.Normal(
                              scale=1 / math.sqrt(fc1.shape[1]))))
    fc3 = fluid.layers.fc(input=fc2, size=400, act='relu',
                          param_attr=fluid.ParamAttr(initializer=fluid.initializer.Normal(
                              scale=1 / math.sqrt(fc2.shape[1]))))
    predict = fluid.layers.fc(input=fc3, size=2, act='softmax',
                              param_attr=fluid.ParamAttr(initializer=fluid.initializer.Normal(
                                  scale=1 / math.sqrt(fc3.shape[1]))))

    cost = fluid.layers.cross_entropy(input=predict, label=words[-1])
    avg_cost = fluid.layers.reduce_sum(cost)
    accuracy = fluid.layers.accuracy(input=predict, label=words[-1])
    auc_var, batch_auc_var, auc_states = \
        fluid.layers.auc(input=predict, label=words[-1], num_thresholds=2 ** 12, slide_steps=20)

    fetch_vars = [predict]
    return avg_cost, auc_var, batch_auc_var, train_feed_vars, inference_feed_vars, fetch_vars

def train_loop(args, train_program, feed_vars, loss, auc_var, batch_auc_var,
               trainer_num, trainer_id):
    dataset = reader.CriteoDataset(args.sparse_feature_dim)
    train_reader = paddle.batch(
        paddle.reader.shuffle(
            dataset.train([args.train_data_path], trainer_num, trainer_id),
            buf_size=args.batch_size * 100),
        batch_size=args.batch_size)
    feed_var_names = [var.name for var in feed_vars]

    place = fluid.CPUPlace()
    exe = fluid.Executor(place)

    exe.run(fluid.default_startup_program())
    total_time = 0

    pass_id = 0
    batch_id = 0

    feeder = fluid.DataFeeder(feed_var_names, place)
    for data in train_reader():
        loss_val, auc_val, batch_auc_val = exe.run(fluid.default_main_program(),
                                            feed = feeder.feed(data),
                                            fetch_list=[loss.name, auc_var.name, batch_auc_var.name])
        break

    loss_val = np.mean(loss_val)
    auc_val = np.mean(auc_val)
    batch_auc_val = np.mean(batch_auc_val)

    logger.info("TRAIN --> pass: {} batch: {} loss: {} auc: {}, batch_auc: {}"
                      .format(pass_id, batch_id, loss_val/args.batch_size, auc_val, batch_auc_val))

def save_program():
    args = parse_args()

    if not os.path.isdir(args.model_output_dir):
        os.mkdir(args.model_output_dir)

    loss, auc_var, batch_auc_var, train_feed_vars, inference_feed_vars, fetch_vars = ctr_dnn_model(args.embedding_size, args.sparse_feature_dim, use_py_reader=False)

    optimizer = fluid.optimizer.Adam(learning_rate=1e-4)
    optimizer.minimize(loss)
    main_program = fluid.default_main_program()

    place = fluid.CPUPlace()
    exe = fluid.Executor(place)

    train_loop(args, main_program, train_feed_vars, loss, auc_var, batch_auc_var, 1, 0)
    model_dir = args.model_output_dir + "/inference_only"
    feed_var_names = [var.name for var in inference_feed_vars]
    fluid.io.save_inference_model(model_dir, feed_var_names, fetch_vars, exe, fluid.default_main_program())

def prune_program():
    args = parse_args()
    model_dir = args.model_output_dir + "/inference_only"
    model_file = model_dir + "/__model__"
    with open(model_file, "rb") as f:
        protostr = f.read()
    f.close()
    proto = framework_pb2.ProgramDesc.FromString(six.binary_type(protostr))
    block = proto.blocks[0]
    kept_ops = [op for op in block.ops if op.type != "lookup_table"]
    del block.ops[:]
    block.ops.extend(kept_ops)

    kept_vars = [var for var in block.vars if var.name != "SparseFeatFactors"]
    del block.vars[:]
    block.vars.extend(kept_vars)

    with open(model_file, "wb") as f:
        f.write(proto.SerializePartialToString())
    f.close()
    with open(model_file + ".prototxt.pruned", "w") as f:
        f.write(text_format.MessageToString(proto))
    f.close()

def remove_embedding_param_file():
    args = parse_args()
    model_dir = args.model_output_dir + "/inference_only"
    embedding_file = model_dir + "/SparseFeatFactors"
    os.remove(embedding_file)

if __name__ == '__main__':
    save_program()
    prune_program()
    remove_embedding_param_file()
