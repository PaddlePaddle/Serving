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

#include "core/predictor/framework/prometheus_metric.h"

#include <thread>
#include "prometheus/detail/utils.h"
#include "prometheus/counter.h"
#include "prometheus/exposer.h"
#include "core/predictor/common/inner_common.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

PrometheusMetric::PrometheusMetric()
    : registry_(std::make_shared<prometheus::Registry>()),
      serializer_(new prometheus::TextSerializer()),
      query_success_family_(
          prometheus::BuildCounter()
              .Name("pd_query_request_success_total")
              .Help("Number of successful query requests")
              .Register(*registry_)),
      query_failure_family_(
          prometheus::BuildCounter()
              .Name("pd_query_request_failure_total")
              .Help("Number of failed query requests")
              .Register(*registry_)),
      inf_count_family_(prometheus::BuildCounter()
                            .Name("pd_inference_count_total")
                            .Help("Number of inferences performed")
                            .Register(*registry_)),
      query_duration_us_family_(
          prometheus::BuildCounter()
              .Name("pd_query_request_duration_us_total")
              .Help("Cummulative query request duration in microseconds")
              .Register(*registry_)),
      inf_duration_us_family_(
          prometheus::BuildCounter()
              .Name("pd_inference_duration_us_total")
              .Help("Cummulative inference duration in microseconds")
              .Register(*registry_)),
      metrics_enabled_(false)
{
}

size_t
PrometheusMetric::HashLabels(const std::map<std::string, std::string>& labels)
{
  return prometheus::detail::hash_labels(labels);
}

PrometheusMetric::~PrometheusMetric()
{
}

bool
PrometheusMetric::Enabled()
{
  auto singleton = GetSingleton();
  return singleton->metrics_enabled_;
}

void
PrometheusMetric::EnableMetrics()
{
  auto singleton = GetSingleton();
  singleton->metrics_enabled_ = true;
  if (!singleton->exposer_) {
    std::string str_port = std::to_string(FLAGS_prometheus_port);
    std::string url = "127.0.0.1:" + str_port;
    singleton->exposer_ = std::make_shared<prometheus::Exposer>(url);
    singleton->exposer_->RegisterCollectable(PrometheusMetric::GetRegistry());
  }
}

std::shared_ptr<prometheus::Registry>
PrometheusMetric::GetRegistry()
{
  auto singleton = PrometheusMetric::GetSingleton();
  return singleton->registry_;
}

const std::string
PrometheusMetric::SerializedMetrics()
{
  auto singleton = PrometheusMetric::GetSingleton();
  return singleton->serializer_->Serialize(
      singleton->registry_.get()->Collect());
}

PrometheusMetric*
PrometheusMetric::GetSingleton()
{
  static PrometheusMetric singleton;
  return &singleton;
}

PrometheusMetricManager* 
PrometheusMetricManager::GetGeneralSingleton() {
  static PrometheusMetricManager manager("general", 0, -1);
  return &manager;
}


PrometheusMetricManager::PrometheusMetricManager(
    const std::string& model_name, const int64_t model_version,
    const int device)
{
  std::map<std::string, std::string> labels;
  GetMetricLabels(&labels, model_name, model_version, device);

  metric_query_success_ =
      CreateCounterMetric(PrometheusMetric::FamilyQuerySuccess(), labels);
  metric_query_failure_ =
      CreateCounterMetric(PrometheusMetric::FamilyQueryFailure(), labels);
  metric_inf_count_ =
      CreateCounterMetric(PrometheusMetric::FamilyInferenceCount(), labels);
  metric_query_duration_us_ =
      CreateCounterMetric(PrometheusMetric::FamilyQueryDuration(), labels);
  metric_inf_duration_us_ =
      CreateCounterMetric(PrometheusMetric::FamilyInferenceDuration(), labels);
}

PrometheusMetricManager::~PrometheusMetricManager()
{
  PrometheusMetric::FamilyQuerySuccess().Remove(metric_query_success_);
  PrometheusMetric::FamilyQueryFailure().Remove(metric_query_failure_);
  PrometheusMetric::FamilyInferenceCount().Remove(metric_inf_count_);
  PrometheusMetric::FamilyQueryDuration().Remove(metric_query_duration_us_);
  PrometheusMetric::FamilyInferenceDuration().Remove(
      metric_inf_duration_us_);
}

void
PrometheusMetricManager::GetMetricLabels(
    std::map<std::string, std::string>* labels, const std::string& model_name,
    const int64_t model_version, const int device)
{
  labels->insert(std::map<std::string, std::string>::value_type(
      std::string("model"), model_name));
  labels->insert(std::map<std::string, std::string>::value_type(
      std::string("version"), std::to_string(model_version)));

  if (device >= 0) {
    std::string gpu = std::to_string(device);
    labels->insert(std::map<std::string, std::string>::value_type(
       std::string("gpu"), gpu));
  }
}

prometheus::Counter*
PrometheusMetricManager::CreateCounterMetric(
    prometheus::Family<prometheus::Counter>& family,
    const std::map<std::string, std::string>& labels)
{
  return &family.Add(labels);
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
