// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
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

#pragma once

#ifdef BCLOUD
#include <base/containers/flat_map.h>  // FlatMap
#include <base/memory/singleton.h>     // DefaultSingletonTraits
#include <base/scoped_lock.h>          // BAIDU_SCOPED_LOCK
#else
#include <butil/containers/flat_map.h>  // FlatMap
#include <butil/memory/singleton.h>     // DefaultSingletonTraits
#include <butil/scoped_lock.h>          // BAIDU_SCOPED_LOCK
#endif

#include <bvar/bvar.h>  // bvar
#include <string>

#ifdef BCLOUD
namespace butil = base;
#endif

namespace baidu {
namespace paddle_serving {
namespace predictor {

static const char* WORKFLOW_METRIC_PREFIX = "workflow_";
static const char* STAGE_METRIC_PREFIX = "stage_";
static const char* OP_METRIC_PREFIX = "op_";
static const char* NAME_DELIMITER = "_";

typedef ::bvar::Window<::bvar::Adder<int>> AdderWindow;
typedef ::bvar::Window<::bvar::IntRecorder> RecorderWindow;

class AdderWindowMetric {
 public:
  AdderWindowMetric() : sum_window(&sum, ::bvar::FLAGS_bvar_dump_interval) {}

  explicit AdderWindowMetric(const std::string& name)
      : sum_window(
            name + "_sum_window", &sum, ::bvar::FLAGS_bvar_dump_interval) {}

  inline AdderWindowMetric& operator<<(int count) {
    sum << count;
    return *this;
  }

 public:
  ::bvar::Adder<int> sum;
  AdderWindow sum_window;
};

static float g_get_rate(void* arg);
class RateBaseMetric {
 public:
  explicit RateBaseMetric(const std::string& name)
      : rate_value(name + "_rate", g_get_rate, this) {}

  void update_lhs(int count) { lhs.sum << count; }

  void update_rhs(int count) { rhs.sum << count; }

 public:
  ::bvar::PassiveStatus<float> rate_value;
  AdderWindowMetric lhs;
  AdderWindowMetric rhs;
};

static float g_get_rate(void* arg) {
  RateBaseMetric* rate_metric = static_cast<RateBaseMetric*>(arg);
  if (rate_metric->rhs.sum_window.get_value() <= 0) {
    return 0;
  }
  return rate_metric->lhs.sum_window.get_value() * 100 /
         static_cast<float>(rate_metric->rhs.sum_window.get_value());
}

// 计算平均值时取整
class AvgWindowMetric {
 public:
  AvgWindowMetric() : avg_window(&avg, ::bvar::FLAGS_bvar_dump_interval) {}

  explicit AvgWindowMetric(const std::string& name)
      : avg_window(
            name + "_avg_window", &avg, ::bvar::FLAGS_bvar_dump_interval) {}

  inline AvgWindowMetric& operator<<(int64_t value) {
    avg << value;
    return *this;
  }

 public:
  ::bvar::IntRecorder avg;
  RecorderWindow avg_window;
};

// 计算平均值时不取整
static double g_get_double_avg(void* arg);
class AvgDoubleWindowMetric {
 public:
  explicit AvgDoubleWindowMetric(const std::string& name)
      : avg_value(name + "_avg_double_window", g_get_double_avg, this) {}

  inline AvgDoubleWindowMetric& operator<<(int64_t value) {
    recorder << value;
    return *this;
  }

 public:
  ::bvar::PassiveStatus<double> avg_value;
  AvgWindowMetric recorder;
};

static double g_get_double_avg(void* arg) {
  AvgDoubleWindowMetric* avg_metric = static_cast<AvgDoubleWindowMetric*>(arg);
  return avg_metric->recorder.avg_window.get_value().get_average_double();
}

class PredictorMetric {
 public:
  static PredictorMetric* GetInstance();

  ~PredictorMetric() {
    for (::butil::FlatMap<std::string, bvar::LatencyRecorder*>::iterator iter =
             latency_recorder_map.begin();
         iter != latency_recorder_map.end();
         ++iter) {
      delete iter->second;
    }
    for (::butil::FlatMap<std::string, AdderWindowMetric*>::iterator iter =
             adder_window_map.begin();
         iter != adder_window_map.end();
         ++iter) {
      delete iter->second;
    }
    for (::butil::FlatMap<std::string, AvgWindowMetric*>::iterator iter =
             avg_window_map.begin();
         iter != avg_window_map.end();
         ++iter) {
      delete iter->second;
    }
    for (::butil::FlatMap<std::string, AvgDoubleWindowMetric*>::iterator iter =
             avg_double_window_map.begin();
         iter != avg_double_window_map.end();
         ++iter) {
      delete iter->second;
    }
    for (::butil::FlatMap<std::string, RateBaseMetric*>::iterator iter =
             rate_map.begin();
         iter != rate_map.end();
         ++iter) {
      delete iter->second;
    }
  }

  void regist_latency_metric(const std::string& metric_name) {
    {
      BAIDU_SCOPED_LOCK(_mutex);
      LOG(INFO) << "try to regist latency metric[" << metric_name << "].";
      if (latency_recorder_map.seek(metric_name) == NULL) {
        bvar::LatencyRecorder* metric =
            new (std::nothrow) bvar::LatencyRecorder(metric_name);
        latency_recorder_map.insert(metric_name, metric);
        LOG(INFO) << "succ to regist latency metric[" << metric_name << "].";
      }
    }
  }

  void regist_adder_window_metric(const std::string& metric_name) {
    {
      BAIDU_SCOPED_LOCK(_mutex);
      LOG(INFO) << "try to regist adder window metric[" << metric_name << "].";
      if (adder_window_map.seek(metric_name) == NULL) {
        AdderWindowMetric* metric =
            new (std::nothrow) AdderWindowMetric(metric_name);
        adder_window_map.insert(metric_name, metric);
        LOG(INFO) << "succ to regist adder window metric[" << metric_name
                  << "].";
      }
    }
  }

  void regist_avg_window_metric(const std::string& metric_name) {
    {
      BAIDU_SCOPED_LOCK(_mutex);
      LOG(INFO) << "try to regist avg window metric[" << metric_name << "].";
      if (avg_window_map.seek(metric_name) == NULL) {
        AvgWindowMetric* metric =
            new (std::nothrow) AvgWindowMetric(metric_name);
        avg_window_map.insert(metric_name, metric);
        LOG(INFO) << "succ to regist avg window metric[" << metric_name << "].";
      }
    }
  }

  void regist_avg_double_window_metric(const std::string& metric_name) {
    {
      BAIDU_SCOPED_LOCK(_mutex);
      LOG(INFO) << "try to regist avg double window metric[" << metric_name
                << "].";
      if (avg_double_window_map.seek(metric_name) == NULL) {
        AvgDoubleWindowMetric* metric =
            new (std::nothrow) AvgDoubleWindowMetric(metric_name);
        avg_double_window_map.insert(metric_name, metric);
        LOG(INFO) << "succ to regist avg double window metric[" << metric_name
                  << "].";
      }
    }
  }

  void regist_rate_metric(const std::string& metric_name) {
    {
      BAIDU_SCOPED_LOCK(_mutex);
      LOG(INFO) << "try to regist rate metric[" << metric_name << "].";
      if (rate_map.seek(metric_name) == NULL) {
        RateBaseMetric* metric = new (std::nothrow) RateBaseMetric(metric_name);
        rate_map.insert(metric_name, metric);
        LOG(INFO) << "succ to regist rate metric[" << metric_name << "].";
      }
    }
  }

  inline void update_latency_metric(const std::string& metric_name,
                                    int64_t latency) {
    bvar::LatencyRecorder** metric = latency_recorder_map.seek(metric_name);
    if (metric != NULL) {
      **metric << latency;
    } else {
      LOG(ERROR) << "impossible, check if you regist[" << metric_name << "].";
    }
  }

  inline void update_adder_window_metric(const std::string& metric_name,
                                         int count) {
    AdderWindowMetric** metric = adder_window_map.seek(metric_name);
    if (metric != NULL) {
      **metric << count;
    } else {
      LOG(ERROR) << "impossible, check if you regist[" << metric_name << "].";
    }
  }

  inline void update_avg_window_metric(const std::string& metric_name,
                                       int64_t value) {
    AvgWindowMetric** metric = avg_window_map.seek(metric_name);
    if (metric != NULL) {
      **metric << value;
    } else {
      LOG(ERROR) << "impossible, check if you regist[" << metric_name << "].";
    }
  }

  inline void update_avg_double_window_metric(const std::string& metric_name,
                                              int64_t value) {
    AvgDoubleWindowMetric** metric = avg_double_window_map.seek(metric_name);
    if (metric != NULL) {
      **metric << value;
    } else {
      LOG(ERROR) << "impossible, check if you regist[" << metric_name << "].";
    }
  }

  inline void update_rate_metric_lhs(const std::string& name, int count) {
    RateBaseMetric** metric = rate_map.seek(name);
    if (metric != NULL) {
      (*metric)->update_lhs(count);
    } else {
      LOG(ERROR) << "impossible, check if you regist[" << name << "].";
    }
  }

  inline void update_rate_metric_rhs(const std::string& name, int count) {
    RateBaseMetric** metric = rate_map.seek(name);
    if (metric != NULL) {
      (*metric)->update_rhs(count);
    } else {
      LOG(ERROR) << "impossible, check if you regist[" << name << "].";
    }
  }

 private:
  PredictorMetric() : bucket_count(300) {
    latency_recorder_map.init(bucket_count);
    adder_window_map.init(bucket_count);
    avg_window_map.init(bucket_count);
    avg_double_window_map.init(bucket_count);
    rate_map.init(bucket_count);
  }

 private:
  const size_t bucket_count;
  ::butil::FlatMap<std::string, bvar::LatencyRecorder*> latency_recorder_map;
  ::butil::FlatMap<std::string, AdderWindowMetric*> adder_window_map;
  ::butil::FlatMap<std::string, AvgWindowMetric*> avg_window_map;
  ::butil::FlatMap<std::string, AvgDoubleWindowMetric*> avg_double_window_map;
  ::butil::FlatMap<std::string, RateBaseMetric*> rate_map;

  friend struct DefaultSingletonTraits<PredictorMetric>;
  mutable butil::Mutex _mutex;
  DISALLOW_COPY_AND_ASSIGN(PredictorMetric);
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
