// #include "base/iobuf.h"
#include "op/dense_op.h"
#include "framework/memory.h"
#include "framework/infer.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::predictor::native_tensor::DenseTensor;
using baidu::paddle_serving::predictor::native_tensor::DenseRequest;
using baidu::paddle_serving::predictor::native_tensor::DenseResponse;
using baidu::paddle_serving::predictor::native_tensor::TensorType;
using baidu::paddle_serving::predictor::InferManager;

bool dt2pt(TensorType in, paddle::PaddleDType& out, size_t& sz) {
    switch (in) {
        case TensorType::FLOAT:
            out = paddle::PaddleDType::FLOAT32;
            sz = sizeof(float);
            return true;
        case TensorType::DOUBLE:
            return false;
        case TensorType::INT32:
            return false;
        case TensorType::INT64:
            out = paddle::PaddleDType::INT64;
            sz = sizeof(int64_t);
            return true;
        case TensorType::UINT32:
            return false;
        case TensorType::UINT64:
            return false;
        default:
            return false;
    }
}

bool pt2dt(paddle::PaddleDType in, DenseTensor* out, size_t& sz) {
    switch (in) {
        case paddle::PaddleDType::FLOAT32:
            out->set_type(TensorType::FLOAT);
            sz = sizeof(float);
            return true;
        case paddle::PaddleDType::INT64:
            out->set_type(TensorType::INT64);
            sz = sizeof(int64_t);
            return true;
        default:
            return false;
    }
}

int DenseOp::inference() {
    _in.clear();
    _out.clear();

    const DenseRequest* req =
            dynamic_cast<const DenseRequest*>(get_request_message());
    if (!req) {
        LOG(ERROR) << "Failed get dense request message";
        return -1;
    }
    DenseResponse* res = mutable_data<DenseResponse>();
    if (!res) {
        LOG(ERROR) << "Failed get tls output object failed";
        return -1;
    }

    uint32_t tensor_size = req->tensors_size();
    if (tensor_size <= 0) {
        LOG(INFO) << "No samples need to to predicted";
        return -1;
    }
    for (uint32_t ti = 0; ti < tensor_size; ++ti) {
        paddle::PaddleTensor pt;
        const DenseTensor& dt = req->tensors(ti);
        size_t data_size = 1;
        if (!dt2pt(dt.type(), pt.dtype, data_size)) {
            LOG(ERROR) << "Invalid data type: " << dt.type();
            return -1;
        }
        pt.name = dt.name();
        size_t dim_size = 1;
        for (uint32_t si = 0; si < dt.shape_size(); ++si) {
            pt.shape.push_back(dt.shape(si));
            dim_size *= dt.shape(si);
        }
        data_size *= dim_size;

        void* data = 
            baidu::paddle_serving::predictor::MempoolWrapper::instance().malloc(data_size);
        if (!data) {
            LOG(ERROR) << "Failed malloc buffer: " << data_size; 
            return -1;
        }
        for (uint32_t fi = 0; fi < dim_size; ++fi) {
            AppendHelerWrapper::append1(dt.type(), dt, data, fi);
        }
        pt.data = paddle::PaddleBuf(data, data_size);
        _in.push_back(pt);
    }

    // call paddle fluid model for inferencing
    if (InferManager::instance().infer(
                BUILTIN_DENSE_FORMAT_MODEL_NAME, &_in, &_out)) {
        LOG(ERROR) << "Failed do infer in fluid model: " 
            << BUILTIN_DENSE_FORMAT_MODEL_NAME;
        return -1;
    }

    // copy output tensor into response
    for (uint32_t si = 0; si < _out.size(); si++) {
        const paddle::PaddleTensor& pt = _out[si];
        DenseTensor* dt = res->add_tensors();
        if (!dt) {
            LOG(ERROR) << "Failed append new out tensor"; 
            return -1;
        }

        size_t data_size = 1;
        if (!pt2dt(pt.dtype, dt, data_size)) {
            LOG(ERROR) << "Invalid data type: " << pt.dtype;
            return -1;
        }
        dt->set_name(pt.name);

        uint32_t shape_size = pt.shape.size();
        size_t dim_size = 1;
        for (uint32_t si = 0; si < shape_size; ++si) {
            dt->add_shape(pt.shape[si]);
            dim_size *= pt.shape[si];
        }

        // assign output data
        const void* data = pt.data.data();
        for (uint32_t di = 0; di < dim_size; ++di) {
            AppendHelerWrapper::append2(dt->type(), data, dt, di);
        }
    }

    LOG(INFO) << "Response in builtin dense format:"
        << "length:" << res->ByteSize() << ","
        << "data:" << res->ShortDebugString() << ","
        << "in: " << _in.size() << ","
        << "out: " << _out.size();

    // release out tensor object resource
    size_t in_size = _in.size();
    for (size_t ii = 0; ii < in_size; ++ii) {
        _in[ii].shape.clear();
    }
    _in.clear();
    size_t out_size = _out.size();
    for (size_t oi = 0; oi < out_size; ++oi) {
        _out[oi].shape.clear();
    }
    _out.clear();

    return 0; 
}

DEFINE_OP(DenseOp);

} // serving
} // paddle_serving
} // baidu
