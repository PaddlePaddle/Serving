#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>

#include "sw/redis++/redis++.h"

std::string host = "127.0.0.0";
int port = 6379;
std::string auth;
std::string cluster_node;
int cluster_port = 0;
bool benchmark = false;
int key_len = 8;
int val_len = 40;
int total_request_num = 1000;
int thread_num = 1;
int pool_size = 5;
int batch_size = 100;
int key_size = 10000000;        // keys in redis server

sw::redis::Redis *redis;

int parse_options(int argc, char **argv)
{
    int opt = 0;
    while ((opt = getopt(argc, argv, "h:p:a:n:c:k:v:r:t:b:s:")) != -1) {
        try {
            switch (opt) {
            case 'h':
                host = optarg;
                break;

            case 'p':
                port = std::stoi(optarg);
                break;

            case 'a':
                auth = optarg;
                break;

            case 'n':
                cluster_node = optarg;
                break;

            case 'c':
                cluster_port = std::stoi(optarg);
                break;

            case 'b':
                batch_size = std::stoi(optarg);
                break;

            case 'k':
                key_len = std::stoi(optarg);
                break;

            case 'v':
                val_len = std::stoi(optarg);
                break;

            case 'r':
                total_request_num = std::stoi(optarg);
                break;

            case 't':
                thread_num = std::stoi(optarg);
                break;

            case 's':
                pool_size = std::stoi(optarg);
                break;

            default:
                break;
            }
        } catch (const sw::redis::Error &e) {
            std::cerr << "Unknow option" << std::endl;
        } catch (const std::exception &e) {
            std::cerr << "Invalid command line option" << std::endl;
        }
    }

    return 0;
}

void thread_worker()
{
    // get values
    std::vector<std::string> get_kvs;
    std::vector<std::string> get_kvs_res;
    get_kvs_res.reserve(total_request_num);
    uint64_t time_us = 0;

    for (int i = 0; i < total_request_num; ++i) {
        get_kvs.clear();
        get_kvs_res.clear();

        for(int j = i * batch_size; j <  (i + 1) * batch_size; j++) {
            get_kvs.push_back(std::to_string(i % key_size));
        }
        auto start2 = std::chrono::steady_clock::now();
        redis->mget(get_kvs.begin(), get_kvs.end(), get_kvs_res.begin());
        auto stop2 = std::chrono::steady_clock::now();
        time_us += std::chrono::duration_cast<std::chrono::microseconds>(stop2 - start2).count();
    }

    std::cout << total_request_num << " requests, " << batch_size << " keys per req, total time us = " << time_us <<std::endl;
    std::cout << "Average " << time_us / total_request_num << "us per req" << std::endl;
}

int main(int argc, char **argv)
{
    parse_options(argc, argv);

    std::string connstr = std::string("tcp://") + host + std::string(":") + std::to_string(port);
    redis = new sw::redis::Redis(connstr);

    std::vector<std::thread> workers;

    for (int i = 0; i < thread_num; ++i) {
        workers.push_back(std::thread(thread_worker));
    }

    for (int i = 0; i < thread_num; ++i) {
        workers[i].join();
    }
    return 0;
}
