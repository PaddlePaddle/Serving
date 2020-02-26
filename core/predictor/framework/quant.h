#include <fstream>
#include <iostream>
#include <memory>
#include "framework.pb.h"
#include <cstring>
#include <string>
#include <cmath>
#include "ThreadPool.h"
#include "seq_file.h"
#include <mutex>
using paddle::framework::proto::VarType;
void reg_var_types();
void greedy_search(float* in, float &xmin, float &xmax, float& loss, size_t emb_size, int bits);
std::mutex g_mtx;

float compute_loss(float* a, float* b, int emb_size);
float* transfer(float* in, float* out, float min, float max, int emb_size, int bits);
char* quant(float* in, char* out, float min, float max, int emb_size, int bits);
float* dequant(char* in, float* out, float min, float max, int emb_size, int bits);
void greedy_search(float* in, float &xmin, float &xmax, float& loss, size_t emb_size, int bits);
void reg_var_types();
int writeSeq(const char* file1, const char* file2, int bits, int thread_pool_size);

