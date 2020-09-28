/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file op_context.h
 * @author yinyijun@baidu.com
 * @date 2020-06-03
 **/

#ifndef BAIDU_CVPRE_OPCONTEXT_H
#define BAIDU_CVPRE_OPCONTEXT_H

#include <npp.h>

const size_t CHANNEL_SIZE = 3;

// The context as input/ouput of all operators
// contains pointer to raw data on gpu, frame size
class OpContext {
public:
    OpContext() {
        _step = 0;
        _size = 0;
        _p_frame = nullptr;
    }
    // constructor to apply gpu memory of image raw data
    OpContext(int height, int width) {
        _step = sizeof(Npp32f) * width * CHANNEL_SIZE;
        _length = height * width * CHANNEL_SIZE;
        _size = _step * height;
        _nppi_size.height = height;
        _nppi_size.width = width;
        cudaMalloc((void **)(&_p_frame), _size);
    }
    virtual ~OpContext() {
        free_memory();
    }

public:
    Npp32f* p_frame() const {
        return _p_frame;
    }
    int step() const {
        return _step;
    }
    int length() const {
        return _length;
    }
    int size() const {
        return _size;
    }
    NppiSize& nppi_size() {
        return _nppi_size;
    }
    void free_memory() {
        if (_p_frame != nullptr) {
            cudaFree(_p_frame);
            _p_frame = nullptr;
        }
        _nppi_size.height = 0;
        _nppi_size.width = 0;
        _step = 0;
        _size = 0;
    }

private:
    Npp32f *_p_frame;       // pointer to raw data on gpu
    int _step;              // number of bytes in a row
    int _length;            // length of _p_frame, _size = _length * sizeof(Npp32f)
    int _size;              // number of bytes of the image
    NppiSize _nppi_size;    // contains height and width
};

#endif // BAIDU_CVPRE_OPCONTEXT_H