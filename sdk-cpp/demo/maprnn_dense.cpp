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
#include <bthread.h>
#include "common.h"
#include "predictor_sdk.h"
#include "map_rnn.pb.h"
using baidu::paddle_serving::sdk_cpp::Predictor;
using baidu::paddle_serving::sdk_cpp::PredictorApi;
//using baidu::infinite::map_rnn::Tensor;
using baidu::infinite::map_rnn::DenseInstance;
using baidu::infinite::map_rnn::DensePrediction;
using baidu::infinite::map_rnn::Request;
using baidu::infinite::map_rnn::Response;
using baidu::infinite::map_rnn::MapRnnService;
static const uint32_t SELECT_VALID_UNIT = 1000;

int split(std::string source, char spliter, std::vector<std::string>& result)
{
    result.clear();
    std::string::size_type  pos;
    std::string::size_type  start = 0;
    while ((pos = source.find(spliter, start)) != std::string::npos)
    {
        result.insert(result.end(), source.substr(start, pos-start));
        start = pos+1;
    }
    result.insert(result.end(), source.substr(start));
    return (int)result.size();
}

int load_data(std::string data_file_name, std::vector<std::vector<float> >& data){
    std::ifstream data_file;
    std::vector<std::string> token;
    data_file.open(data_file_name, std::ios::in); 
    std::string input_line;
    while (std::getline(data_file, input_line)) {
        split(input_line, ',', token);
        std::vector<float> feature_one;
        for (size_t i = 0; i < token.size(); i++){
            feature_one.push_back(std::stof(token[i]));
        }
        data.push_back(feature_one);
    }
    return 0;
}

void split(const std::string &str, char sep, std::vector<std::string> *pieces) {
    pieces->clear();
    if (str.empty()) {
        return;
    }
    size_t pos = 0;
    size_t next = str.find(sep, pos);
    while (next != std::string::npos) {
        pieces->push_back(str.substr(pos, next - pos));
        pos = next + 1;
        next = str.find(sep, pos);
    }
    if (!str.substr(pos).empty()) {
        pieces->push_back(str.substr(pos));
    }
}

void split_to_float(const std::string &str, char sep, std::vector<float> *fs) {
    std::vector<std::string> pieces;
    split(str, sep, &pieces);
    std::transform(pieces.begin(), pieces.end(), std::back_inserter(*fs),
            [](const std::string &v) { 
                return std::stof(v); 
            });
}

// clang-format off
/*void TensorAssignData(paddle::PaddleTensor *tensor, const std::vector<std::vector<float>> &data) {
  // Assign buffer
  int dim = std::accumulate(tensor->shape.begin(), tensor->shape.end(), 1, [](int a, int b) { return a * b; });
  tensor->data.Resize(sizeof(float) * dim);
  int c = 0;
  for (const auto &f : data) {
    for (float v : f) { static_cast<float *>(tensor->data.data())[c++] = v; }
  }
}*/
// clang-format on

struct DataRecord {
    std::vector<std::vector<std::vector<float>>> link_step_data_all;
    std::vector<std::vector<float>> week_data_all, minute_data_all;
    std::vector<std::vector<std::vector<char>>> ch_link_step_data_all;  
    std::vector<std::vector<char>> ch_week_data_all, ch_minute_data_all;

    std::vector<size_t> lod1, lod2, lod3;
    std::vector<std::vector<float>> rnn_link_data, rnn_week_datas,
        rnn_minute_datas;

    size_t batch_iter{0};
    size_t batch_size{1};

    DataRecord() = default;
    DataRecord(const std::string &path, int batch_size = 1)
        : batch_size(batch_size) {
            Load(path);

        for (std::vector<std::vector<std::vector<float>>>::iterator it1 = link_step_data_all.begin();
                it1 != link_step_data_all.end(); ++it1) {
            std::vector<std::vector<char>> links;
            for (std::vector<std::vector<float>>::iterator it2 = it1->begin(); it2 != it1->end(); ++it2) {
                int len = it2->size() * sizeof(float);
                char* ch = (char*)malloc(len);
                memcpy(ch, it2->data(), len);
                std::vector<char> tmp(ch, ch + len);
                links.push_back(tmp);
                free(ch);
            }
            ch_link_step_data_all.push_back(links);
        }

        for (std::vector<std::vector<float>>::iterator it1 = week_data_all.begin(); it1 != week_data_all.end(); ++it1) {
            int len = it1->size() * sizeof(float);
            char* ch = (char*)malloc(len);
            memcpy(ch, it1->data(), len);
            std::vector<char> tmp(ch, ch + len);
            ch_week_data_all.push_back(tmp);
            free(ch);
        }

        for (std::vector<std::vector<float>>::iterator it1 = minute_data_all.begin(); it1 != minute_data_all.end(); ++it1) {
            int len = it1->size() * sizeof(float);
            char* ch = (char*)malloc(len);
            memcpy(ch, it1->data(), len);
            std::vector<char> tmp(ch, ch + len);
            ch_minute_data_all.push_back(tmp);
            free(ch);
        }
    }

    DataRecord NextBatch() {
        DataRecord data;
        size_t batch_end = batch_iter + batch_size;

        // NOTE skip the final batch, if no enough data is provided.
        if (batch_end <= link_step_data_all.size()) {
            data.link_step_data_all.assign(link_step_data_all.begin() + batch_iter,
                    link_step_data_all.begin() + batch_end);
            data.week_data_all.assign(week_data_all.begin() + batch_iter,
                    week_data_all.begin() + batch_end);
            data.minute_data_all.assign(minute_data_all.begin() + batch_iter,
                    minute_data_all.begin() + batch_end);

            // Prepare LoDs
            data.lod1.emplace_back(0);
            data.lod2.emplace_back(0);
            data.lod3.emplace_back(0);

            //CHECK(!data.link_step_data_all.empty()) << "empty";
            //CHECK(!data.week_data_all.empty());
            //CHECK(!data.minute_data_all.empty());
            //CHECK_EQ(data.link_step_data_all.size(), data.week_data_all.size());
            //CHECK_EQ(data.minute_data_all.size(), data.link_step_data_all.size());

            for (size_t j = 0; j < data.link_step_data_all.size(); j++) {
                for (const auto &d : data.link_step_data_all[j]) {
                    data.rnn_link_data.push_back(d);
                }
                data.rnn_week_datas.push_back(data.week_data_all[j]);
                data.rnn_minute_datas.push_back(data.minute_data_all[j]);
                // calculate lod
                data.lod1.push_back(data.lod1.back() +
                        data.link_step_data_all[j].size());
                data.lod3.push_back(data.lod3.back() + 1);
                for (size_t i = 1; i < data.link_step_data_all[j].size() + 1; i++) {
                    data.lod2.push_back(data.lod2.back() +
                            data.link_step_data_all[j].size());
                }
            }
        }

        batch_iter += batch_size;
        return data;
    }

    void Load(const std::string &path) {
        std::ifstream file(path);
        std::string line;

        int num_lines = 0;
        while (std::getline(file, line)) {
            num_lines++;
            std::vector<std::string> data;
            split(line, ':', &data);

            std::vector<std::vector<float>> link_step_data;
            std::vector<std::string> link_datas;
            split(data[0], '|', &link_datas);

            for (auto &step_data : link_datas) {
                std::vector<float> tmp;
                split_to_float(step_data, ',', &tmp);
                link_step_data.emplace_back(tmp);
            }

            // load week data
            std::vector<float> week_data;
            split_to_float(data[2], ',', &week_data);

            // load minute data
            std::vector<float> minute_data;
            split_to_float(data[1], ',', &minute_data);

            link_step_data_all.emplace_back(std::move(link_step_data));
            week_data_all.emplace_back(std::move(week_data));
            minute_data_all.emplace_back(std::move(minute_data));
        }
    }
};

/*void PrepareInputs(std::vector<paddle::PaddleTensor> *input_slots, DataRecord *data,
                   int batch_size) {
  // DataRecord data(FLAGS_datapath, batch_size);

  paddle::PaddleTensor lod_attention_tensor, init_zero_tensor, lod_tensor_tensor,
      week_tensor, minute_tensor;
  lod_attention_tensor.name = "lod_attention";
  init_zero_tensor.name = "init_zero";
  lod_tensor_tensor.name = "lod_tensor";
  week_tensor.name = "week";
  minute_tensor.name = "minute";

  auto one_batch = data->NextBatch();

    printf("rnn_link_data.size:%lu,\n", one_batch.rnn_link_data.size());
    printf("rnn_link_data.front().size:%lu\n", one_batch.rnn_link_data.front().size());

  // clang-format off
  std::vector<int> rnn_link_data_shape
      ({static_cast<int>(one_batch.rnn_link_data.size()), static_cast<int>(one_batch.rnn_link_data.front().size())});
  //LOG(INFO) << "set 1";
  lod_attention_tensor.shape.assign({1, 2});
  lod_attention_tensor.lod.assign({one_batch.lod1, one_batch.lod2});
  //LOG(INFO) << "set 1";
  init_zero_tensor.shape.assign({batch_size, 15});
  init_zero_tensor.lod.assign({one_batch.lod3});
  //LOG(INFO) << "set 1";
  lod_tensor_tensor.shape = rnn_link_data_shape;
  lod_tensor_tensor.lod.assign({one_batch.lod1});
  //LOG(INFO) << "set 1";
  week_tensor.shape.assign({(int) one_batch.rnn_week_datas.size(), (int) one_batch.rnn_week_datas.front().size()});
  week_tensor.lod.assign({one_batch.lod3});
  //LOG(INFO) << "set 1";
  minute_tensor.shape.assign({(int) one_batch.rnn_minute_datas.size(),
                              (int) one_batch.rnn_minute_datas.front().size()});
  minute_tensor.lod.assign({one_batch.lod3});

  // assign data
  TensorAssignData(&lod_attention_tensor, std::vector<std::vector<float>>({{0, 0}}));
  std::vector<float> tmp_zeros(batch_size * 15, 0.);
  TensorAssignData(&init_zero_tensor, {tmp_zeros});
  TensorAssignData(&lod_tensor_tensor, one_batch.rnn_link_data);
  TensorAssignData(&week_tensor, one_batch.rnn_week_datas);
  TensorAssignData(&minute_tensor, one_batch.rnn_minute_datas);
  // clang-format on

  input_slots->assign({lod_tensor_tensor, lod_attention_tensor,
                       init_zero_tensor, init_zero_tensor, week_tensor,
                       minute_tensor});

  for (auto &tensor : *input_slots) {
    tensor.dtype = paddle::PaddleDType::FLOAT32;
    // LOG(INFO) << DescribeTensor(tensor);
  }
}*/

class InputData {
public:
    InputData() {}
    ~InputData() {}
    int create(const std::string file_name, size_t buf_size, 
            size_t batch_size, int qps) {
        bthread_mutex_init(&_mutex, NULL);
        std::string datapath = "./data/test_features_sys";
        DataRecord data(datapath, batch_size);
        _data_record = data;
        /*FILE* fp = fopen(file_name.c_str(), "r");
        if (!fp) {
            LOG(FATAL) << "Failed open data file: " 
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
        */
        for (size_t ri = 0; ri < buf_size; ri++) {
            Request* req = new Request();
            if (generate_one_req(*req, batch_size) != 0) {
                LOG(ERROR) << "Failed generate req at: " << ri;
                //fclose(fp);
                return -1;
            }
            _req_list.push_back(req);
        }
        //fclose(fp);
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
        bthread_mutex_lock(&_mutex);
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
        bthread_mutex_unlock(&_mutex);
        return req;
    }
    int generate_one_req(Request& req, int batch) {
        int batch_size = batch;
        
        int i = 0;
        DenseInstance* ins = req.add_instances();
        ins->set_batch_size(batch_size);        
        for (std::vector<std::vector<std::vector<char>>>::iterator it1 = _data_record.ch_link_step_data_all.begin();
                it1 != _data_record.ch_link_step_data_all.end(); ++it1) {
            ::baidu::infinite::map_rnn::Lines* step_data = ins->add_step_data();
            for (std::vector<std::vector<char>>::iterator it2 = it1->begin(); it2 != it1->end(); ++it2) {
                ::baidu::infinite::map_rnn::Line* line = step_data->add_line();        
                line->set_value(it2->data(), it2->size());
            }
            if (++i == batch_size) {
                break;
            }
        }
        i = 0;
        ::baidu::infinite::map_rnn::Lines* week_data = ins->mutable_week_data();
        for (std::vector<std::vector<char>>::iterator it1 = _data_record.ch_week_data_all.begin(); 
                it1 != _data_record.ch_week_data_all.end(); ++it1) {
            ::baidu::infinite::map_rnn::Line* line = week_data->add_line();
            line->set_value(it1->data(), it1->size());
            if (++i == batch_size) {
                break;
            }
        }
        i = 0;
        ::baidu::infinite::map_rnn::Lines* minute_data = ins->mutable_minute_data();
        for (std::vector<std::vector<char>>::iterator it1 = _data_record.ch_minute_data_all.begin(); 
                it1 != _data_record.ch_minute_data_all.end(); ++it1) {
            ::baidu::infinite::map_rnn::Line* line = minute_data->add_line();
            line->set_value(it1->data(), it1->size());
            if (++i == batch_size) {
                break;
            }
        }

        /*for (int fi = 0; fi < _data.size(); ++fi) {
            Tensor* tensor = ins->add_tensors();
            tensor->set_name(tensor_names[fi]);
            int len = 1;
            for (int si = 0; si < shapes[fi].size(); ++si) {
                len *= shapes[fi][si];
            }
            for (int si = 0; si < shapes[fi].size(); ++si) {
                tensor->add_shape(shapes[fi][si]);
            }
            tensor->set_features(&(_data[fi][0]), len * sizeof(float));
        }*/
        return 0;
    }
private:
    DataRecord _data_record;
    std::vector<std::vector<float> > _data;
    std::vector<Request*> _req_list;
    bthread_mutex_t _mutex;
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
    if (PredictorApi::instance().thrd_initialize() != 0) {
        LOG(ERROR) << "Failed create bthread local predictor"; 
        return NULL;
    }
    Response res;
    LOG(WARNING) << "Thread entry!";
    while (true) {
        if (PredictorApi::instance().thrd_clear() != 0) {
            LOG(ERROR) << "Failed thrd clear predictor"; 
            return NULL;
        }
        Predictor* predictor = PredictorApi::instance().fetch_predictor("wasq");
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
        if (PredictorApi::instance().free_predictor(predictor) != 0) {
            printf("failed free predictor\n");
        }
        //break;
        //printf("done\n");
    }
    if (PredictorApi::instance().thrd_finalize() != 0) {
        LOG(ERROR) << "Failed thrd finalize predictor api";
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
    if (PredictorApi::instance().create("./conf", "predictors.conf") != 0) {
        LOG(ERROR) << "Failed create predictors api!"; 
        return -1;
    }
    InputData data;
    if (data.create(
                "./data/test_features_sys", req_buffer, batch_size, qps) != 0) {
        LOG(ERROR) << "Failed create inputdata!";
        return -1;
    }
    Arg arg = {NULL, &data};
    bthread_t* threads = new bthread_t[thread_num];
    if (!threads) {
        LOG(ERROR) << "Failed create threads, num:" << thread_num;
        return -1;
    }
    for (int i = 0; i < thread_num; ++i) {
        bthread_start_background(threads + i, NULL, work, &arg);
    }
    for (int i = 0; i < thread_num; ++i) {
        bthread_join(threads[i], NULL);
    }
    delete[] threads;
    data.destroy();
    return 0;
}
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
