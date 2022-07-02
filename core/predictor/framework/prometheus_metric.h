// Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
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
//
#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include "prometheus/registry.h"
#include "prometheus/serializer.h"
#include "prometheus/text_serializer.h"
#include "prometheus/counter.h"

namespace prometheus {
  class Exposer;
}

namespace baidu {
namespace paddle_serving {
namespace predictor {

class PrometheusMetric {
 public:
  static size_t HashLabels(const std::map<std::string, std::string>& labels);

  static bool Enabled();

  static void EnableMetrics();

  static std::shared_ptr<prometheus::Registry> GetRegistry();

  static const std::string SerializedMetrics();

  static prometheus::Family<prometheus::Counter>& FamilyQuerySuccess()
  {
    return GetSingleton()->query_success_family_;
  }

  static prometheus::Family<prometheus::Counter>& FamilyQueryFailure()
  {
    return GetSingleton()->query_failure_family_;
  }

  static prometheus::Family<prometheus::Counter>& FamilyInferenceCount()
  {
    return GetSingleton()->inf_count_family_;
  }

  static prometheus::Family<prometheus::Counter>& FamilyQueryDuration()
  {
    return GetSingleton()->query_duration_us_family_;
  }

  static prometheus::Family<prometheus::Counter>& FamilyInferenceDuration()
  {
    return GetSingleton()->inf_duration_us_family_;
  }

 private:
  PrometheusMetric();
  virtual ~PrometheusMetric();
  static PrometheusMetric* GetSingleton();

  std::shared_ptr<prometheus::Registry> registry_;
  std::unique_ptr<prometheus::Serializer> serializer_;
  std::shared_ptr<prometheus::Exposer> exposer_;

  prometheus::Family<prometheus::Counter>& query_success_family_;
  prometheus::Family<prometheus::Counter>& query_failure_family_;
  prometheus::Family<prometheus::Counter>& inf_count_family_;
  prometheus::Family<prometheus::Counter>& query_duration_us_family_;
  prometheus::Family<prometheus::Counter>& inf_duration_us_family_;

  bool metrics_enabled_;
};

class PrometheusMetricManager {
 public:

  static PrometheusMetricManager* GetGeneralSingleton();

  ~PrometheusMetricManager();

  prometheus::Counter& MetricQuerySuccess() const
  {
    return *metric_query_success_;
  }
  prometheus::Counter& MetricQueryFailure() const
  {
    return *metric_query_failure_;
  }
  prometheus::Counter& MetricInferenceCount() const
  {
    return *metric_inf_count_;
  }
  prometheus::Counter& MetricQueryDuration() const
  {
    return *metric_query_duration_us_;
  }
  prometheus::Counter& MetricInferenceDuration() const
  {
    return *metric_inf_duration_us_;
  }

 private:
  PrometheusMetricManager(
      const std::string& model_name, const int64_t model_version,
      const int device);

  static void GetMetricLabels(
      std::map<std::string, std::string>* labels, const std::string& model_name,
      const int64_t model_version, const int device);
  prometheus::Counter* CreateCounterMetric(
      prometheus::Family<prometheus::Counter>& family,
      const std::map<std::string, std::string>& labels);

  prometheus::Counter* metric_query_success_;
  prometheus::Counter* metric_query_failure_;
  prometheus::Counter* metric_inf_count_;
  prometheus::Counter* metric_query_duration_us_;
  prometheus::Counter* metric_inf_duration_us_;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
