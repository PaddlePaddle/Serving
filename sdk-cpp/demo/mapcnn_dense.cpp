/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
/**
 * @file demo.cpp
 * @author root(root@baidu.com)
 * @date 2018/07/09 20:12:44
 * @brief 
 *  
 **/
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "common.h"
#include "predictor_sdk.h"
#include "map_cnn.pb.h"
using baidu::paddle_serving::sdk_cpp::Predictor;
using baidu::paddle_serving::sdk_cpp::PredictorApi;
using baidu::infinite::map_model::SparseTensor;
using baidu::infinite::map_model::SparseInstance;
using baidu::infinite::map_model::DensePrediction;
using baidu::infinite::map_model::Request;
using baidu::infinite::map_model::Response;
using baidu::infinite::map_model::MapCnnService;
static const uint32_t SELECT_VALID_UNIT = 1000;
class InputData {
public:
    InputData() {}
    ~InputData() {}
    int create(const std::string file_name, size_t buf_size, 
            size_t batch_size, int qps) {
        pthread_mutex_init(&_mutex, NULL);
        FILE* fp = fopen(file_name.c_str(), "r");
        if (!fp) {
            LOG(ERROR) << "Failed open data file: " 
                << file_name;
            return -1;
        } 
        _data.clear();
        char buffer[2048000];
        std::vector<std::string> tokens;
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            tokens.clear();
            baidu::paddle_serving::sdk_cpp::str_split(
                    buffer, ",", &tokens);
            std::vector<float> feature_one;
            for (size_t i = 0; i < tokens.size(); i++){
                feature_one.push_back(
                        strtof(tokens[i].c_str(), NULL));
            }
            _data.push_back(feature_one);
        }
        printf("succ load data, size:%ld\n", _data.size());
        for (size_t ri = 0; ri < buf_size; ri++) {
            Request* req = new Request();
            if (generate_one_req(*req, batch_size) != 0) {
                LOG(ERROR) << "Failed generate req at: " << ri;
                fclose(fp);
                return -1;
            }
            _req_list.push_back(req);
        }
        fclose(fp);
        _current = 0;
        _waitingtm = 0;
        _lasttm.tv_sec = _lasttm.tv_usec = 0;
        if (qps == 0) {
            _interval = 0;
        } else if (qps < 1) {
            _interval = 1000 * 1000;
        } else {
            _interval = 1000 * 1000 / qps;
        }
        LOG(INFO) << "Succ create req, size: " << buf_size
            << ", batch_size: " << batch_size;
        return 0;
    }
    void destroy() {
        size_t ds = _data.size();
        for (size_t di = 0; di < ds; di++) {
            _data[di].clear();
        }
        _data.clear();
        size_t rs = _req_list.size();
        for (size_t ri = 0; ri < rs; ri++) {
            delete _req_list[ri];
        }
        _req_list.clear();
    }
    Request* next_req() {
        pthread_mutex_lock(&_mutex);
        if (_interval != 0)
        {
            if (_lasttm.tv_sec == 0 && _lasttm.tv_usec == 0)
            {
                gettimeofday(&_lasttm, NULL);
            }
            else
            {
                timeval curtm;
                gettimeofday(&curtm, NULL);
                long elapse = 
                    ((curtm.tv_sec - _lasttm.tv_sec) * 1000*1000 +
                    (curtm.tv_usec - _lasttm.tv_usec));
                _waitingtm += _interval - elapse;
                _lasttm = curtm;
                if (_waitingtm >= SELECT_VALID_UNIT) // select的最小响应单位
                {
                    long tm_unit 
                        = _waitingtm / SELECT_VALID_UNIT * SELECT_VALID_UNIT;
                    timeval tmp_tm = {tm_unit / 1000000, tm_unit % 1000000};
                    select(1, NULL, NULL, NULL, &tmp_tm); //延时以控制压力speed
                }
                else if (_waitingtm <= SELECT_VALID_UNIT * (-2))
                {
                    _waitingtm = -SELECT_VALID_UNIT;
                }
            }
        }
        size_t rs = _req_list.size();
        Request* req = _req_list[(_current++) % rs];
        pthread_mutex_unlock(&_mutex);
        return req;
    }
    int generate_one_req(Request& req, int batch) {
        int batch_size = batch;
        std::vector<std::vector<int> > shapes;
        shapes.clear();
        int p_shape[] = {batch_size, 37, 1, 1};
        std::vector<int> shape(p_shape, p_shape + 4);
        shapes.push_back(shape);
        int p_shape1[] = {batch_size, 1, 50, 12};
        std::vector<int> shape1(p_shape1, p_shape1 + 4);
        shapes.push_back(shape1);
        int p_shape2[] = {batch_size, 1, 50, 19};
        std::vector<int> shape2(p_shape2, p_shape2 + 4);
        shapes.push_back(shape2);
        int p_shape3[] = {batch_size, 1, 50, 1};
        std::vector<int> shape3(p_shape3, p_shape3 + 4);
        shapes.push_back(shape3);
        int p_shape4[] = {batch_size, 4, 50, 1};
        std::vector<int> shape4(p_shape4, p_shape4 + 4);
        shapes.push_back(shape4);
        int p_shape5[] = {batch_size, 1, 50, 1};
        std::vector<int> shape5(p_shape5, p_shape5 + 4);
        shapes.push_back(shape5);
        int p_shape6[] = {batch_size, 5, 50, 1};
        std::vector<int> shape6(p_shape6, p_shape6 + 4);
        shapes.push_back(shape6);
        int p_shape7[] = {batch_size, 7, 50, 1};
        std::vector<int> shape7(p_shape7, p_shape7 + 4);
        shapes.push_back(shape7);
        int p_shape8[] = {batch_size, 3, 50, 1};
        std::vector<int> shape8(p_shape8, p_shape8 + 4);
        shapes.push_back(shape8);
        int p_shape9[] = {batch_size, 32, 50, 1}; // added
        std::vector<int> shape9(p_shape9, p_shape9 + 4);
        shapes.push_back(shape9);
        

        std::vector<std::string> tensor_names;
        tensor_names.push_back("input_0");
        tensor_names.push_back("input_1");
        tensor_names.push_back("input_2");
        tensor_names.push_back("input_3");
        tensor_names.push_back("input_4");
        tensor_names.push_back("input_5");
        tensor_names.push_back("input_6");
        tensor_names.push_back("input_7");
        tensor_names.push_back("input_8");
        tensor_names.push_back("input_9");

        SparseInstance* ins = req.add_instances();
        for (int fi = 0; fi < _data.size(); ++fi) {
            SparseTensor* tensor = ins->add_tensors();
            tensor->set_name(tensor_names[fi]);
            int len = 1;
            for (int si = 0; si < shapes[fi].size(); ++si) {
                len *= shapes[fi][si];
            }
            for (int si = 0; si < shapes[fi].size(); ++si) {
                tensor->add_shape(shapes[fi][si]);
            }
            tensor->set_features(&(_data[fi][0]), len * sizeof(float));
        }
        return 0;
    }
private:
    std::vector<std::vector<float> > _data;
    std::vector<Request*> _req_list;
    pthread_mutex_t _mutex;
    long _waitingtm;
    long _interval;
    timeval _lasttm;
    int _current;
};
void print_res(
        const Request* req, 
        const Response& res, 
        std::string route_tag,
        uint64_t elapse_ms) {
    uint32_t sample_size = res.predictions_size();
    for (int i = 0; i < sample_size; ++i) {
        const ::baidu::infinite::map_model::DensePrediction& prediction = res.predictions(i);
        int cat_size = prediction.categories_size();
        for (int j = 0; j < cat_size; ++j) {
            //LOG(INFO) << "categories:" << prediction.categories(j);
        }
    }
    LOG(INFO) 
        << "Succ call predictor[wasq], res sample size: " 
        << sample_size << ", the tag is: " 
        << route_tag << ", elapse_ms: " << elapse_ms;
}
struct Arg {
    PredictorApi* api;
    InputData* input;
};
void* work(void* p) {
    Arg* arg = (Arg*) p;
    InputData* input = arg->input;
    PredictorApi* api = arg->api;
    Response res;
    LOG(WARNING) << "Thread entry!";
    while (true) {
        Predictor* predictor = api->fetch_predictor("mapcnn");
        if (!predictor) {
            LOG(ERROR) << "Failed fetch predictor: wasq"; 
            return NULL;
        }
        Request* req = input->next_req();
        res.Clear();
        timeval start;
        gettimeofday(&start, NULL);
        if (predictor->inference(req, &res) != 0) {
            LOG(ERROR) << "failed call predictor with req:"
                << req->ShortDebugString();
            return NULL;
        }
        timeval end;
        gettimeofday(&end, NULL);
        uint64_t elapse_ms = (end.tv_sec * 1000 + end.tv_usec / 1000)
            - (start.tv_sec * 1000 + start.tv_usec / 1000);
        print_res(req, res, predictor->tag(), elapse_ms);
        if (api->free_predictor(predictor) != 0) {
            printf("failed free predictor\n");
        }
    }
    LOG(WARNING) << "Thread exit!";
    return NULL;
}
int main(int argc, char** argv) {
    if (argc != 5) {
        printf("Usage: demo req_buf_size batch_size threads qps\n");
        return -1;
    }
    int req_buffer = atoi(argv[1]);
    int batch_size = atoi(argv[2]);
    int thread_num = atoi(argv[3]);
    int qps = atoi(argv[4]);
    PredictorApi api;
    if (api.create("./conf", "predictors.conf") != 0) {
        LOG(ERROR) << "Failed create predictors api!"; 
        return -1;
    }
    InputData data;
    if (data.create(
                "./data/pure_feature", req_buffer, batch_size, qps) != 0) {
        LOG(ERROR) << "Failed create inputdata!";
        return -1;
    }
    Arg arg = {&api, &data};
    pthread_t* threads = new pthread_t[thread_num];
    if (!threads) {
        LOG(ERROR) << "Failed create threads, num:" << thread_num;
        return -1;
    }
    for (int i = 0; i < thread_num; ++i) {
        pthread_create(threads + i, NULL, work, &arg);
    }
    for (int i = 0; i < thread_num; ++i) {
        pthread_join(threads[i], NULL);
    }
    delete[] threads;
    data.destroy();
    return 0;
}
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
