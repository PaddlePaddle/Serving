#pragma once

#include "native_tensor.pb.h"
#include "common/inner_common.h"
#include "op/op.h"
#include "framework/channel.h"
#include "framework/op_repository.h"
#include "paddle/fluid/inference/paddle_inference_api.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

static const char* BUILTIN_DENSE_FORMAT_MODEL_NAME
    = "image_classification_resnet";

class DenseOp : public baidu::paddle_serving::predictor::OpWithChannel<
      baidu::paddle_serving::predictor::native_tensor::DenseResponse> {
public: 
    typedef std::vector<paddle::PaddleTensor> TensorVector;

    DECLARE_OP(DenseOp);

    int inference();

private:
    TensorVector _in;
    TensorVector _out;
};

template<baidu::paddle_serving::predictor::native_tensor::TensorType type>
class AppendHelper {
public:
    typedef baidu::paddle_serving::predictor::native_tensor::DenseTensor DenseTensor;
    static void append1(const DenseTensor& src, void* dst, uint32_t index) {
        LOG(ERROR) << "Not implement append1 for type: " << type;
    }

    static void append2(const void* src, DenseTensor* dst, uint32_t index) {
        LOG(ERROR) << "Not implement append2 for type:" << type;
    }
};

template<>
class AppendHelper<baidu::paddle_serving::predictor::native_tensor::TensorType::FLOAT> {
public:
    typedef baidu::paddle_serving::predictor::native_tensor::DenseTensor DenseTensor;
    static void append1(const DenseTensor& src, void* dst, uint32_t index) {
        ((float*)dst)[index] = src.float_data(index);
    }

    static void append2(const void* src, DenseTensor* dst, uint32_t index) {
        dst->add_float_data(((float*)src)[index]);
    }
};

template<>
class AppendHelper<baidu::paddle_serving::predictor::native_tensor::TensorType::DOUBLE> {
public:
    typedef baidu::paddle_serving::predictor::native_tensor::DenseTensor DenseTensor;
    static void append1(const DenseTensor& src, void* dst, uint32_t index) {
        ((double*)dst)[index] = src.double_data(index);
    }

    static void append2(const void* src, DenseTensor* dst, uint32_t index) {
        dst->add_double_data(((double*)dst)[index]);
    }
};

template<>
class AppendHelper<baidu::paddle_serving::predictor::native_tensor::TensorType::UINT32> {
public:
    typedef baidu::paddle_serving::predictor::native_tensor::DenseTensor DenseTensor;
    static void append1(const DenseTensor& src, void* dst, uint32_t index) {
        ((uint32_t*)dst)[index] = src.uint32_data(index);
    }

    static void append2(const void* src, DenseTensor* dst, uint32_t index) {
        dst->add_uint32_data(((uint32_t*)dst)[index]);
    }
};

template<>
class AppendHelper<baidu::paddle_serving::predictor::native_tensor::TensorType::UINT64> {
public:
    typedef baidu::paddle_serving::predictor::native_tensor::DenseTensor DenseTensor;
    static void append1(const DenseTensor& src, void* dst, uint32_t index) {
        ((uint64_t*)dst)[index] = src.uint64_data(index);
    }

    static void append2(const void* src, DenseTensor* dst, uint32_t index) {
        dst->add_uint64_data(((uint64_t*)dst)[index]);
    }
};

template<>
class AppendHelper<baidu::paddle_serving::predictor::native_tensor::TensorType::INT32> {
public:
    typedef baidu::paddle_serving::predictor::native_tensor::DenseTensor DenseTensor;
    static void append1(const DenseTensor& src, void* dst, uint32_t index) {
        ((int32_t*)dst)[index] = src.int32_data(index);
    }

    static void append2(const void* src, DenseTensor* dst, uint32_t index) {
        dst->add_int32_data(((int32_t*)dst)[index]);
    }
};

template<>
class AppendHelper<baidu::paddle_serving::predictor::native_tensor::TensorType::INT64> {
public:
    typedef baidu::paddle_serving::predictor::native_tensor::DenseTensor DenseTensor;
    static void append1(const DenseTensor& src, void* dst, uint32_t index) {
        ((int64_t*)dst)[index] = src.int64_data(index);
    }

    static void append2(const void* src, DenseTensor* dst, uint32_t index) {
        dst->add_int64_data(((int64_t*)dst)[index]);
    }
};

class AppendHelerWrapper {
public:
    typedef baidu::paddle_serving::predictor::native_tensor::DenseTensor DenseTensor;
    static void append1(
            baidu::paddle_serving::predictor::native_tensor::TensorType type,
            const DenseTensor& src, void* dst, uint32_t index) {
        switch (type) {
        case baidu::paddle_serving::predictor::native_tensor::TensorType::FLOAT:
            AppendHelper<
                    baidu::paddle_serving::predictor::native_tensor::TensorType::FLOAT
                >::append1(src, dst, index);
            break;
        case baidu::paddle_serving::predictor::native_tensor::TensorType::DOUBLE:
            AppendHelper<
                    baidu::paddle_serving::predictor::native_tensor::TensorType::DOUBLE
                >::append1(src, dst, index);
            break;
        case baidu::paddle_serving::predictor::native_tensor::TensorType::UINT32:
            AppendHelper<
                    baidu::paddle_serving::predictor::native_tensor::TensorType::UINT32
                >::append1(src, dst, index);
            break;
        case baidu::paddle_serving::predictor::native_tensor::TensorType::UINT64:
            AppendHelper<
                    baidu::paddle_serving::predictor::native_tensor::TensorType::UINT64
                >::append1(src, dst, index);
            break;
        case baidu::paddle_serving::predictor::native_tensor::TensorType::INT32:
            AppendHelper<
                    baidu::paddle_serving::predictor::native_tensor::TensorType::INT32
                >::append1(src, dst, index);
            break;
        case baidu::paddle_serving::predictor::native_tensor::TensorType::INT64:
            AppendHelper<
                    baidu::paddle_serving::predictor::native_tensor::TensorType::INT64
                >::append1(src, dst, index);
            break;
        default:
            ;
        } 
    }

    static void append2(
            baidu::paddle_serving::predictor::native_tensor::TensorType type,
            const void* src, DenseTensor* dst, uint32_t index) {
        switch (type) {
        case baidu::paddle_serving::predictor::native_tensor::TensorType::FLOAT:
            AppendHelper<
                    baidu::paddle_serving::predictor::native_tensor::TensorType::FLOAT
                >::append2(src, dst, index);
            break;
        case baidu::paddle_serving::predictor::native_tensor::TensorType::DOUBLE:
            AppendHelper<
                    baidu::paddle_serving::predictor::native_tensor::TensorType::DOUBLE
                >::append2(src, dst, index);
            break;
        case baidu::paddle_serving::predictor::native_tensor::TensorType::UINT32:
            AppendHelper<
                    baidu::paddle_serving::predictor::native_tensor::TensorType::UINT32
                >::append2(src, dst, index);
            break;
        case baidu::paddle_serving::predictor::native_tensor::TensorType::UINT64:
            AppendHelper<
                    baidu::paddle_serving::predictor::native_tensor::TensorType::UINT64
                >::append2(src, dst, index);
            break;
        case baidu::paddle_serving::predictor::native_tensor::TensorType::INT32:
            AppendHelper<
                    baidu::paddle_serving::predictor::native_tensor::TensorType::INT32
                >::append2(src, dst, index);
            break;
        case baidu::paddle_serving::predictor::native_tensor::TensorType::INT64:
            AppendHelper<
                    baidu::paddle_serving::predictor::native_tensor::TensorType::INT64
                >::append2(src, dst, index);
            break;
        default:
            ;
        } 
    }
};

} // serving
} // paddle_serving
} // baidu
