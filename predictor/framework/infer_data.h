#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_INFER_DATA_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_INFER_DATA_H

#include "common/inner_common.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

enum DataType {
    FLOAT32,
    INT64
};

class DataBuf {
public:
    DataBuf() : _data(NULL), _size(0), _owned(true) {}

    DataBuf(size_t size) 
            : _data(new char[size]), _size(size), _owned(true) {}

    DataBuf(void* data, size_t size)
            : _data(data), _size(size), _owned(false) {}

    DataBuf(void* data, size_t size, bool owned)
            : _data(data), _size(size), _owned(owned) {}

    void* data() const {
        return _data;
    }

    size_t size() const {
        return _size;
    }

    void free() {
        _size = 0;
        if (_owned) {
            delete[] (char*)_data;
        }
    }

    ~DataBuf() {
        free();
    }

private:
    void* _data;
    size_t _size;
    bool _owned;
};

struct Tensor {
    Tensor() {
        shape.clear();
        for (int li = 0; li < lod.size(); ++li) {
            lod[li].clear();
        }
        lod.clear();
    }

    Tensor(const Tensor& tensor) {
        name = tensor.name; 
        data = tensor.data;
        type = tensor.type;
        shape.assign(tensor.shape.begin(), tensor.shape.end());
        for (int li = 0; li < tensor.lod.size(); ++li) {
            std::vector<size_t> l;
            l.assign(tensor.lod[li].begin(), tensor.lod[li].end());
            lod.push_back(l);
        }
    }

    ~Tensor() {
        shape.clear();
    }

    size_t ele_byte() const {
        if (type == INT64) {
            return sizeof(int64_t);
        } else {
            return sizeof(float);
        }
    }

    bool valid() const {
        if (shape.empty()) {
            if (data.data() || data.size()) {
                LOG(ERROR) << "data should be empty";
                return false;
            }
            return true;
        }

        if (!data.data() || !data.size()) {
            LOG(ERROR) << "data cannot empty";
            return false;
        }

        size_t byte_size = 1;
        for (size_t si = 0; si < shape.size(); ++si) {
            byte_size *= shape[si];
        } 

        if (byte_size * ele_byte() != data.size()) {
            LOG(ERROR) << "wrong data size: " << byte_size * ele_byte() << " vs. " << data.size();
            return false;
        }

        return true;
    }

    size_t shape0() {
        if (shape.empty()) {
            return 0;
        }
        return shape[0];
    }

    std::string name;
    std::vector<int> shape;
    DataBuf data;
    DataType type;
    std::vector<std::vector<size_t> > lod;
};

class BatchTensor {
public:
    BatchTensor() {}
    ~BatchTensor() {
        _features.clear();
    }

    BatchTensor(const BatchTensor& tv) {
        _features.assign(
                tv.features().begin(), tv.features().end());
    }

    Tensor& operator[](int index) {
        return _features[index];
    }

    const Tensor& operator[](int index) const {
        return _features[index];
    }

    void push_back(const Tensor& tensor) {
        _features.push_back(tensor);
    }

    size_t count() const {
        return _features.size();
    }

    size_t size() const {
        // shape0 indicates batch_size
        if (count() <= 0 || _features[0].shape.size() <= 0) {
            return 0;
        }
        return _features[0].shape[0];
    }

    const std::vector<Tensor>& features() const {
        return _features;
    }

    void clear() {
        _features.clear();
    }

private:
    std::vector<Tensor> _features;
};

} // predictor
} // paddle_serving
} // baidu

#endif // BAIDU_PADDLE_SERVING_PREDICTOR_INFER_DATA_H
