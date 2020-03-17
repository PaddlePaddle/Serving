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

import paddle.fluid as fluid
import math


def dnn_model(dense_input, sparse_inputs, label, embedding_size,
              sparse_feature_dim):
    def embedding_layer(input):
        emb = fluid.layers.embedding(
            input=input,
            is_sparse=True,
            is_distributed=False,
            size=[sparse_feature_dim, embedding_size],
            param_attr=fluid.ParamAttr(
                name="SparseFeatFactors",
                initializer=fluid.initializer.Uniform()))
        return fluid.layers.sequence_pool(input=emb, pool_type='sum')

    def mlp_input_tensor(emb_sums, dense_tensor):
        if isinstance(dense_tensor, fluid.Variable):
            return fluid.layers.concat(emb_sums, axis=1)
        else:
            return fluid.layers.concat(emb_sums + [dense_tensor], axis=1)

    def mlp(mlp_input):
        fc1 = fluid.layers.fc(input=mlp_input,
                              size=400,
                              act='relu',
                              param_attr=fluid.ParamAttr(
                                  initializer=fluid.initializer.Normal(
                                      scale=1 / math.sqrt(mlp_input.shape[1]))))
        fc2 = fluid.layers.fc(input=fc1,
                              size=400,
                              act='relu',
                              param_attr=fluid.ParamAttr(
                                  initializer=fluid.initializer.Normal(
                                      scale=1 / math.sqrt(fc1.shape[1]))))
        fc3 = fluid.layers.fc(input=fc2,
                              size=400,
                              act='relu',
                              param_attr=fluid.ParamAttr(
                                  initializer=fluid.initializer.Normal(
                                      scale=1 / math.sqrt(fc2.shape[1]))))
        pre = fluid.layers.fc(input=fc3,
                              size=2,
                              act='softmax',
                              param_attr=fluid.ParamAttr(
                                  initializer=fluid.initializer.Normal(
                                      scale=1 / math.sqrt(fc3.shape[1]))))
        return pre

    emb_sums = list(map(embedding_layer, sparse_inputs))
    mlp_in = mlp_input_tensor(emb_sums, dense_input)
    predict = mlp(mlp_in)
    cost = fluid.layers.cross_entropy(input=predict, label=label)
    avg_cost = fluid.layers.reduce_sum(cost)
    accuracy = fluid.layers.accuracy(input=predict, label=label)
    auc_var, batch_auc_var, auc_states = \
        fluid.layers.auc(input=predict, label=label, num_thresholds=2 ** 12, slide_steps=20)
    return predict, avg_cost, auc_var, batch_auc_var
