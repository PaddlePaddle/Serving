#   Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
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
"""
The function lex_net(args) define the lexical analysis network structure
"""
import sys
import os
import math

import paddle.fluid as fluid
from paddle.fluid.initializer import NormalInitializer


def gru_backbone(words, args, target=None):
    word_emb_dim = args.word_emb_dim
    grnn_hidden_dim = args.grnn_hidden_dim
    emb_lr = args.emb_learning_rate if 'emb_learning_rate' in dir(args) else 1.0
    crf_lr = args.emb_learning_rate if 'crf_learning_rate' in dir(args) else 1.0
    bigru_num = args.bigru_num
    init_bound = 0.1
    IS_SPARSE = True

    def _bigru_layer(input_feature):
        """
        define the bidirectional gru layer
        """
        pre_gru = fluid.layers.fc(
            input=input_feature,
            size=grnn_hidden_dim * 3,
            param_attr=fluid.ParamAttr(
                initializer=fluid.initializer.Uniform(
                    low=-init_bound, high=init_bound),
                regularizer=fluid.regularizer.L2DecayRegularizer(
                    regularization_coeff=1e-4)))
        gru = fluid.layers.dynamic_gru(
            input=pre_gru,
            size=grnn_hidden_dim,
            param_attr=fluid.ParamAttr(
                initializer=fluid.initializer.Uniform(
                    low=-init_bound, high=init_bound),
                regularizer=fluid.regularizer.L2DecayRegularizer(
                    regularization_coeff=1e-4)))

        pre_gru_r = fluid.layers.fc(
            input=input_feature,
            size=grnn_hidden_dim * 3,
            param_attr=fluid.ParamAttr(
                initializer=fluid.initializer.Uniform(
                    low=-init_bound, high=init_bound),
                regularizer=fluid.regularizer.L2DecayRegularizer(
                    regularization_coeff=1e-4)))
        gru_r = fluid.layers.dynamic_gru(
            input=pre_gru_r,
            size=grnn_hidden_dim,
            is_reverse=True,
            param_attr=fluid.ParamAttr(
                initializer=fluid.initializer.Uniform(
                    low=-init_bound, high=init_bound),
                regularizer=fluid.regularizer.L2DecayRegularizer(
                    regularization_coeff=1e-4)))

        bi_merge = fluid.layers.concat(input=[gru, gru_r], axis=1)
        return bi_merge

    def _net_conf(word, target=None):
        """
        Configure the network
        """
        word_embedding = fluid.embedding(
            input=word,
            size=[args.vocab_size, args.word_emb_dim],
            dtype='float32',
            is_sparse=IS_SPARSE,
            param_attr=fluid.ParamAttr(
                learning_rate=emb_lr,
                name="word_emb",
                initializer=fluid.initializer.Uniform(
                    low=-init_bound, high=init_bound)))

        input_feature = word_embedding
        for i in range(bigru_num):
            bigru_output = _bigru_layer(input_feature)
            input_feature = bigru_output

        emission = fluid.layers.fc(
            size=args.num_labels,
            input=bigru_output,
            param_attr=fluid.ParamAttr(
                initializer=fluid.initializer.Uniform(
                    low=-init_bound, high=init_bound),
                regularizer=fluid.regularizer.L2DecayRegularizer(
                    regularization_coeff=1e-4)))

        if target is not None:
            crf_cost = fluid.layers.linear_chain_crf(
                input=emission,
                label=target,
                param_attr=fluid.ParamAttr(
                    name='crfw', learning_rate=crf_lr))
            avg_cost = fluid.layers.mean(x=crf_cost)
            crf_decode = fluid.layers.crf_decoding(
                input=emission, param_attr=fluid.ParamAttr(name='crfw'))
            return avg_cost, crf_decode

        else:
            size = emission.shape[1]
            fluid.layers.create_parameter(
                shape=[size + 2, size], dtype=emission.dtype, name='crfw')
            crf_decode = fluid.layers.crf_decoding(
                input=emission, param_attr=fluid.ParamAttr(name='crfw'))

        return crf_decode

    crf_decode = None
    if target is None:
        crf_decode = _net_conf(words)
        return crf_decode
    else:
        avg_cost, crf_decode = _net_conf(words, target)
        return avg_cost, crf_decode


class LACNet(object):
    def __init__(self):
        self.model_created = False
        self.inputs = []
        self.outputs = []
        self.args = None

    def set_args(self, args):
        self.args = args

    def create_model(self):
        self.model_created = True
        words = fluid.data(
            name="words", shape=[None, 1], dtype="int64", lod_level=1)
        self.inputs.append(words)

        if self.args.infer == False:
            targets = fluid.data(
                name="targets", shape=[None, 1], dtype="int64", lod_level=1)
            self.inputs.append(targets)

        if self.args.backbone == "bigru":
            if self.args.infer:
                crf_decode = gru_backbone(words, self.args)
                self.outputs.append(crf_decode[0])
            else:
                loss, crf_decode = gru_backbone(words, self.args, targets)
                self.outputs.append(crf_decode)
                self.loss = loss

    def get_inputs(self):
        if self.model_created:
            return self.inputs
        else:
            return []

    def get_input_names(self):
        return [x.name for x in self.inputs]

    def get_outputs(self):
        if self.model_created:
            return self.outputs
        else:
            return []

    def get_loss(self):
        if self.model_created:
            return self.loss
        else:
            return []
