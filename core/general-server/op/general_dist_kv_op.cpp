
#include "core/general-server/op/general_dist_kv_op.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include "core/predictor/framework/infer.h"
#include "core/predictor/framework/memory.h"
#include "core/predictor/framework/resource.h"
#include "core/predictor/framework/kv_manager.h"
#include "core/util/include/timer.h"
#include "cube/cube-api/include/cube_api.h"

namespace baidu {
namespace paddle_serving {
namespace serving {
using baidu::paddle_serving::Timer;
using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::general_model::Response;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::FetchInst;
using baidu::paddle_serving::predictor::InferManager;
using baidu::paddle_serving::predictor::PaddleGeneralModelConfig;


int GeneralDistKVOp::inference() {
  VLOG(2) << "Going to run inference";
  const GeneralBlob *input_blob = get_depend_argument<GeneralBlob>(pre_name());
  VLOG(2) << "Get precedent op name: " << pre_name();
  GeneralBlob *output_blob = mutable_data<GeneralBlob>();

  if (!input_blob) {
    LOG(ERROR) << "Failed mutable depended argument, op:" << pre_name();
    return -1;
  }

  const TensorVector *in = &input_blob->tensor_vector;
  TensorVector* out = &output_blob->tensor_vector;

  int batch_size = input_blob->GetBatchSize();
  VLOG(2) << "input batch size: " << batch_size;
  output_blob->SetBatchSize(batch_size);

  VLOG(2) << "infer batch size: " << batch_size; 

  Timer timeline;
  int64_t start = timeline.TimeStampUS();

  std::vector<std::vector<size_t>> out_lods;
  out_lods.resize(batch_size);

  std::vector<uint64_t> keys;
  // process input blob and fetch the keys to seek
  for (int tensor_idx = 0; tensor_idx < in->size(); ++tensor_idx) {
    out_lods[tensor_idx] = in->at(tensor_idx).lod[0];
    for (int x=0; x < in->at(tensor_idx).shape[0]; ++x) {
      for (int y=0; y < in->at(tensor_idx).shape[1]; ++y) {
        // TODO: may support int32 and more
        uint64_t* key_ptr = static_cast<uint64_t*>(in->at(tensor_idx).data.data());
        keys.push_back(key_ptr[x * in->at(tensor_idx).shape[1] + y]);
      }
    }
  }
  std::string table_name = "test_dict";
  rec::mcube::CubeAPI *cube = rec::mcube::CubeAPI::instance();
  predictor::KVManager &kv_manager = predictor::KVManager::instance();
  const predictor::KVInfo *kvinfo =
      kv_manager.get_kv_info(table_name);
  // add Dist KV Seek Here
  std::vector<rec::mcube::CubeValue> values;
  // TODO: add configure reader here
  int ret = cube->seek(table_name, keys, &values);
 
  std::vector<int64_t> elem_size;
  std::vector<int64_t> capacity;

  int CTR_EMBEDDING_SIZE= 9; 
  for (int tensor_idx = 0; tensor_idx < in->size(); ++tensor_idx) {
    paddle::PaddleTensor lod_tensor;
    lod_tensor.lod.resize(1);
    lod_tensor.lod[0] = out_lods[tensor_idx];
    lod_tensor.dtype = paddle::PaddleDType::FLOAT32;
    lod_tensor.shape = in->at(tensor_idx).shape; 
    VLOG(2) << "var[" << tensor_idx << "] is lod_tensor";
    capacity[tensor_idx] = 1;
    for (int k = 0; in->at(tensor_idx).shape.size(); ++k) {
      int dim = in->at(tensor_idx).shape[k];
      VLOG(2) << "shape for var[" << tensor_idx<< "]: " << dim;
      capacity[tensor_idx] *= dim; 
      lod_tensor.shape.push_back(dim);
    }
    out->push_back(lod_tensor);    
  }

  for (int tensor_idx = 0; tensor_idx < in->size(); ++tensor_idx) {
    out->at(tensor_idx).data.Resize(capacity[tensor_idx]* CTR_EMBEDDING_SIZE *sizeof(float));   
  }
  int cube_val_idx = 0;
  for (int tensor_idx = 0; tensor_idx < in->size(); ++tensor_idx) {
    float *dst_ptr = static_cast<float*>(out->at(tensor_idx).data.data());
    for (int x = 0; x < in->at(tensor_idx).shape[0]; ++x) {
      for (int y = 0; y < in->at(tensor_idx).shape[1]; ++y) {
        float* data_ptr = dst_ptr + x * in->at(tensor_idx).shape[1] * CTR_EMBEDDING_SIZE + y; 
        memcpy(data_ptr, values[cube_val_idx++].buff.data(), values[cube_val_idx++].buff.size()); 
      }
    }
  }

  int64_t end = timeline.TimeStampUS();
  CopyBlobInfo(input_blob, output_blob);
  AddBlobInfo(output_blob, start);
  AddBlobInfo(output_blob, end);  
  return 0;
}
DEFINE_OP(GeneralDistKVOp);

} //namespace serving
} //namespace paddle_serving
} //namespace baidu
