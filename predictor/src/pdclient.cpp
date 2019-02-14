// Copyright (c) 2014 baidu-rpc authors.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// A client sending requests to server every 1 second.
//
#include <fstream>

#include "dense_service.pb.h"
#include "image_classification.pb.h"
#include "sparse_service.pb.h"
#include "int64tensor_service.pb.h"

#include "common/utils.h"
#include "common/inner_common.h"
#include "common/constant.h"
#include "framework/logger.h"

DEFINE_string(attachment, "foo", "Carry this along with requests");
DEFINE_bool(auth, false, "Enable Giano authentication");
DEFINE_string(auth_group, "g_guest", "Giano Group");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in protocol/baidu/rpc/options.proto");
DEFINE_bool(compress, true, "Enable compression");
//DEFINE_string(protocol, "http", "Protocol type. Defined in protocol/baidu/rpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "0.0.0.0:8010", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)"); 
DEFINE_int32(interval_ms, 1000, "Milliseconds between consecutive requests");
DEFINE_string(http_content_type, "application/json", "Content type of http request");

using baidu::paddle_serving::predictor::FLAGS_logger_path;
using baidu::paddle_serving::predictor::FLAGS_logger_file;
using baidu::paddle_serving::predictor::LoggerWrapper;

namespace dense_format {

using baidu::paddle_serving::predictor::dense_service::BuiltinDenseFormatService_Stub;
using baidu::paddle_serving::predictor::dense_service::Request;
using baidu::paddle_serving::predictor::dense_service::Response;

using baidu::paddle_serving::predictor::format::DenseInstance;
using baidu::paddle_serving::predictor::format::DensePrediction;

void send_dense_format(BuiltinDenseFormatService_Stub& stub, int log_id) {
    brpc::Controller cntl;

    // We will receive response synchronously, safe to put variables
    // on stack.
    baidu::paddle_serving::predictor::TimerFlow timer("dense");

    Request dense_request;
    Response dense_response;

    // set request header
    DenseInstance* ins = NULL;
    ins = dense_request.mutable_instances()->Add();
    ins->add_features(1.5);
    ins->add_features(16.0);
    ins->add_features(14.0);
    ins->add_features(23.0);

    timer.check("fill");

    cntl.set_log_id(log_id ++);  // set by user
    if (FLAGS_protocol != "http" && FLAGS_protocol != "h2c")  {
        // Set attachment which is wired to network directly instead of 
        // being serialized into protobuf messages.
        cntl.request_attachment().append(FLAGS_attachment);
    } else {
        cntl.http_request().set_content_type(FLAGS_http_content_type);
    }

    if (FLAGS_compress) {
        cntl.set_request_compress_type(brpc::COMPRESS_TYPE_SNAPPY);
    }

    timer.check("compress");

    // Because `done'(last parameter) is NULL, this function waits until
    // the response comes back or error occurs(including timedout).
    stub.debug(&cntl, &dense_request, &dense_response, NULL);

    timer.check("inference");
    if (!cntl.Failed()) {
        if (cntl.response_attachment().empty()) {
            LOG(INFO) << "Received response from " << cntl.remote_side()
                      << " to " << cntl.local_side()
                      << ": " << dense_response.ShortDebugString()
                      << " latency=" << cntl.latency_us() << "us";
        } else {
            LOG(INFO) << "Received response from " << cntl.remote_side()
                      << " to " << cntl.local_side()
                      << ": " << dense_response.ShortDebugString()
                      << " (attached=" << cntl.response_attachment() << ")"
                      << " latency=" << cntl.latency_us() << "us ";
        }
    } else {
        LOG(WARNING) << cntl.ErrorText();
    }
    timer.check("dump");
}
} // namespace dense_format


namespace sparse_format {

using baidu::paddle_serving::predictor::sparse_service::BuiltinSparseFormatService_Stub;
using baidu::paddle_serving::predictor::sparse_service::Request;
using baidu::paddle_serving::predictor::sparse_service::Response;

using baidu::paddle_serving::predictor::format::SparseInstance;
using baidu::paddle_serving::predictor::format::SparsePrediction;

void send_sparse_format(BuiltinSparseFormatService_Stub& stub, int log_id) {
    brpc::Controller cntl;

    // We will receive response synchronously, safe to put variables
    // on stack.
    baidu::paddle_serving::predictor::TimerFlow timer("sparse");

    Request sparse_request;
    Response sparse_response;

    // set request body
    SparseInstance* ins = NULL;
    ins = sparse_request.mutable_instances()->Add();
    ins->add_keys(26);
    ins->add_keys(182);
    ins->add_keys(232);
    ins->add_keys(243);
    ins->add_keys(431);

    ins->add_shape(2000);

    ins->add_values(1);
    ins->add_values(1);
    ins->add_values(1);
    ins->add_values(4);
    ins->add_values(14);

    ins = sparse_request.mutable_instances()->Add();
    ins->add_keys(0);
    ins->add_keys(182);
    ins->add_keys(232);
    ins->add_keys(243);
    ins->add_keys(431);

    ins->add_shape(2000);

    ins->add_values(13);
    ins->add_values(1);
    ins->add_values(1);
    ins->add_values(4);
    ins->add_values(1);

    timer.check("fill");

    cntl.set_log_id(log_id ++);  // set by user
    if (FLAGS_protocol != "http" && FLAGS_protocol != "h2c")  {
        // Set attachment which is wired to network directly instead of 
        // being serialized into protobuf messages.
        cntl.request_attachment().append(FLAGS_attachment);
    } else {
        cntl.http_request().set_content_type(FLAGS_http_content_type);
    }

    if (FLAGS_compress) {
        cntl.set_request_compress_type(brpc::COMPRESS_TYPE_SNAPPY);
    }

    timer.check("compress");

    // Because `done'(last parameter) is NULL, this function waits until
    // the response comes back or error occurs(including timedout).
    stub.inference(&cntl, &sparse_request, &sparse_response, NULL);

    timer.check("inference");
    if (!cntl.Failed()) {
        if (cntl.response_attachment().empty()) {
            LOG(INFO) << "Received response from " << cntl.remote_side()
                      << " to " << cntl.local_side()
                      << ": " << sparse_response.ShortDebugString()
                      << " latency=" << cntl.latency_us() << "us";
        } else {
            LOG(INFO) << "Received response from " << cntl.remote_side()
                      << " to " << cntl.local_side()
                      << ": " << sparse_response.ShortDebugString()
                      << " (attached=" << cntl.response_attachment() << ")"
                      << " latency=" << cntl.latency_us() << "us";
        }
    } else {
        LOG(WARNING) << cntl.ErrorText();
    }
    timer.check("dump");
}
}

namespace fluid_format {

using baidu::paddle_serving::predictor::int64tensor_service::BuiltinFluidService_Stub;
using baidu::paddle_serving::predictor::int64tensor_service::Request;
using baidu::paddle_serving::predictor::int64tensor_service::Response;

using baidu::paddle_serving::predictor::format::Int64TensorInstance;
using baidu::paddle_serving::predictor::format::Float32TensorPredictor;

void send_fluid_format(BuiltinFluidService_Stub& stub, int log_id) {
    brpc::Controller cntl;

    // We will receive response synchronously, safe to put variables
    // on stack.
    baidu::paddle_serving::predictor::TimerFlow timer("fluid");

    Request fluid_request;
    Response fluid_response;

    // set request header
    Int64TensorInstance* ins = NULL;
    ins = fluid_request.mutable_instances()->Add();
    ins->add_data(15);
    ins->add_data(160);
    ins->add_data(14);
    ins->add_data(23);
    ins->add_data(18);
    ins->add_data(39);
    ins->add_shape(2);
    ins->add_shape(3);

    timer.check("fill");

    cntl.set_log_id(log_id);  // set by user
    if (FLAGS_protocol != "http" && FLAGS_protocol != "h2c")  {
        // Set attachment which is wired to network directly instead of 
        // being serialized into protobuf messages.
        cntl.request_attachment().append(FLAGS_attachment);
    } else {
        cntl.http_request().set_content_type(FLAGS_http_content_type);
    }

    if (FLAGS_compress) {
        cntl.set_request_compress_type(brpc::COMPRESS_TYPE_SNAPPY);
    }

    timer.check("compress");

    // Because `done'(last parameter) is NULL, this function waits until
    // the response comes back or error occurs(including timedout).
    stub.debug(&cntl, &fluid_request, &fluid_response, NULL);

    timer.check("inference");
    if (!cntl.Failed()) {
        if (cntl.response_attachment().empty()) {
            LOG(INFO) << "Received response from " << cntl.remote_side()
                      << " to " << cntl.local_side()
                      << ": " << fluid_response.ShortDebugString()
                      << " latency=" << cntl.latency_us() << "us";
        } else {
            LOG(INFO) << "Received response from " << cntl.remote_side()
                      << " to " << cntl.local_side()
                      << ": " << fluid_response.ShortDebugString()
                      << " (attached=" << cntl.response_attachment() << ")"
                      << " latency=" << cntl.latency_us() << "us ";
        }
    } else {
        LOG(WARNING) << cntl.ErrorText();
    }
    timer.check("dump");
}
} // namespace fluid_format

namespace ximage_format {

char* g_image_buffer = NULL;
size_t g_image_size = 0;
std::string g_image_path = "./data/images/what.jpg";

using baidu::paddle_serving::predictor::image_classification::ImageClassifyService_Stub;
using baidu::paddle_serving::predictor::image_classification::Request;
using baidu::paddle_serving::predictor::image_classification::Response;

using baidu::paddle_serving::predictor::format::XImageReqInstance;
using baidu::paddle_serving::predictor::format::XImageResInstance;

void send_ximage_format(ImageClassifyService_Stub& stub, int log_id) {
    brpc::Controller cntl;

    // We will receive response synchronously, safe to put variables
    // on stack.
    baidu::paddle_serving::predictor::TimerFlow timer("ximage");

    Request ximage_request;
    Response ximage_response;

    // set request header
    std::ifstream fin(g_image_path.c_str(), std::ios::binary);
    fin.seekg(0, std::ios::end);
    int isize = fin.tellg();
    if (g_image_size < isize || !g_image_buffer) {
        g_image_buffer = new (std::nothrow) char[isize];
        g_image_size = isize;
    }

    fin.seekg(0, std::ios::beg);  
    fin.read(g_image_buffer, sizeof(char) * isize);  
    fin.close();

    timer.check("read");

    XImageReqInstance* ins = ximage_request.mutable_instances()->Add();
    ins->set_image_binary(g_image_buffer, isize);
    ins->set_image_length(isize);
    timer.check("fill");

    cntl.set_log_id(log_id ++);  // set by user
    if (FLAGS_protocol != "http" && FLAGS_protocol != "h2c")  {
        // Set attachment which is wired to network directly instead of 
        // being serialized into protobuf messages.
        cntl.request_attachment().append(FLAGS_attachment);
    } else {
        cntl.http_request().set_content_type(FLAGS_http_content_type);
    }

    if (FLAGS_compress) {
        cntl.set_request_compress_type(brpc::COMPRESS_TYPE_SNAPPY);
    }

    timer.check("compress");

    // Because `done'(last parameter) is NULL, this function waits until
    // the response comes back or error occurs(including timedout).
    stub.inference(&cntl, &ximage_request, &ximage_response, NULL);
    timer.check("inference");
    if (!cntl.Failed()) {
        if (cntl.response_attachment().empty()) {
            LOG(INFO) << "Received response from " << cntl.remote_side()
                      << " to " << cntl.local_side()
                      << ": " << ximage_response.ShortDebugString()
                      << " latency=" << cntl.latency_us() << "us";
        } else {
            LOG(INFO) << "Received response from " << cntl.remote_side()
                      << " to " << cntl.local_side()
                      << ": " << ximage_response.ShortDebugString()
                      << " (attached=" << cntl.response_attachment() << ")"
                      << " latency=" << cntl.latency_us() << "us ";
        }
    } else {
        LOG(WARNING) << cntl.ErrorText();
    }
    timer.check("dump");
    if (g_image_buffer) {
        delete[] g_image_buffer;
        g_image_buffer = NULL;
    }

}
} // namespace ximage_format

int main(int argc, char* argv[]) {
    // Parse gflags. We recommend you to use gflags as well.
    google::ParseCommandLineFlags(&argc, &argv, true);

    // initialize logger instance
    if (LoggerWrapper::instance().initialize(
                FLAGS_logger_path, FLAGS_logger_file) != 0) {
        LOG(ERROR) << "Failed initialize logger, conf:" 
            << FLAGS_logger_path << "/" << FLAGS_logger_file;
        return -1;
    }
    
    // Login to get `CredentialGenerator' (see baas-lib-c/baas.h for more
    // information) and then pass it to `GianoAuthenticator'. 
    std::unique_ptr<brpc::policy::GianoAuthenticator> auth;
    if (FLAGS_auth) {
        if (baas::BAAS_Init() != 0) {
            LOG(ERROR) << "Fail to init BAAS";
            return -1;
        }
        baas::CredentialGenerator gen = baas::ClientUtility::Login(FLAGS_auth_group);
        auth.reset(new brpc::policy::GianoAuthenticator(&gen, NULL));
    }

    // A Channel represents a communication line to a Server. Notice that 
    // Channel is thread-safe and can be shared by all threads in your program.
    brpc::Channel channel;
    
    // Initialize the channel, NULL means using default options.
    brpc::ChannelOptions options;
    options.protocol = FLAGS_protocol;
    options.connection_type = FLAGS_connection_type;
    options.auth = auth.get();
    options.timeout_ms = FLAGS_timeout_ms/*milliseconds*/;
    options.max_retry = FLAGS_max_retry;
    if (channel.Init(FLAGS_server.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
        LOG(ERROR) << "Fail to initialize channel";
        return -1;
    }

    // Normally, you should not call a Channel directly, but instead construct
    // a stub Service wrapping it. stub can be shared by all threads as well.
    baidu::paddle_serving::predictor::sparse_service::BuiltinSparseFormatService_Stub
        stub1(&channel);

    baidu::paddle_serving::predictor::dense_service::BuiltinDenseFormatService_Stub
        stub2(&channel);

    baidu::paddle_serving::predictor::int64tensor_service::BuiltinFluidService_Stub
        stub3(&channel);

    baidu::paddle_serving::predictor::image_classification::ImageClassifyService_Stub
        stub4(&channel);

    // Send a request and wait for the response every 1 second.
    int log_id = 0;
    while (!brpc::IsAskedToQuit()) {
        // We will receive response synchronously, safe to put variables
        // on stack.
        log_id++;
        sparse_format::send_sparse_format(stub1, log_id);
        usleep(FLAGS_interval_ms * 1000L);
        log_id++;
        dense_format::send_dense_format(stub2, log_id);
        usleep(FLAGS_interval_ms * 1000L);
        //log_id++;
        //fluid_format::send_fluid_format(stub3, log_id);
        //usleep(FLAGS_interval_ms * 1000L);
        log_id++;
        ximage_format::send_ximage_format(stub4, log_id);
        usleep(FLAGS_interval_ms * 1000L);
    }

    LOG(INFO) << "Pdserving Client is going to quit";
    return 0;
}

