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

from grpc_tools import protoc
import os
import platform


def python_version():
    return [int(v) for v in platform.python_version().split(".")]


def find_package(pkgname):
    try:
        get_distribution(pkgname)
        return True
    except DistributionNotFound:
        return False


def gen_pipeline_code(package_name):
    # pipeline service proto
    protoc.main((
        '',
        '-I.',
        '--python_out=.',
        '--grpc_python_out=.',
        '{}/pipeline/proto/pipeline_service.proto'.format(package_name), ))

    # pipeline grpc-gateway proto
    # *.pb.go
    ret = os.system(
        "cd {}/pipeline/gateway/proto/ && "
        "../../../../../third_party/install/protobuf/bin/protoc -I. "
        "-I$GOPATH/src "
        "-I$GOPATH/src/github.com/grpc-ecosystem/grpc-gateway/third_party/googleapis "
        "--go_out=plugins=grpc:. "
        "gateway.proto".format(package_name))
    if ret != 0:
        exit(1)
    # *.gw.go
    ret = os.system(
        "cd {}/pipeline/gateway/proto/ && "
        "../../../../../third_party/install/protobuf/bin/protoc -I. "
        "-I$GOPATH/src "
        "-I$GOPATH/src/github.com/grpc-ecosystem/grpc-gateway/third_party/googleapis "
        "--grpc-gateway_out=logtostderr=true:. "
        "gateway.proto".format(package_name))
    if ret != 0:
        exit(1)

    # pipeline grpc-gateway shared-lib
    ret = os.system(
        "cd {}/pipeline/gateway && "
        "go build -buildmode=c-shared -o libproxy_server.so proxy_server.go".
        format(package_name))
    if ret != 0:
        exit(1)
