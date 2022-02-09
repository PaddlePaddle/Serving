from prometheus_client import Counter, generate_latest, CollectorRegistry, Gauge

registry = CollectorRegistry()
metric_query_success = Counter("pd_query_request_success_total", "metric_query_success", registry=registry)
metric_query_failure = Counter("pd_query_request_failure_total", "metric_query_failure", registry=registry)
metric_inf_count = Counter("pd_inference_count_total", "metric_inf_count", registry=registry)
metric_query_duration_us = Counter("pd_query_request_duration_us_total", "metric_query_duration_us", registry=registry)
metric_inf_duration_us = Counter("pd_inference_duration_us_total", "metric_inf_duration_us", registry=registry)
