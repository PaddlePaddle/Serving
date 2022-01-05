from prometheus_client import Counter, generate_latest, CollectorRegistry

registry = CollectorRegistry()
metric_query_success = Counter("QuerySucc", "metric_query_success", registry=registry)
metric_query_failure = Counter("QueryFail", "metric_query_failure", registry=registry)
metric_inf_count = Counter("InferCnt", "metric_inf_count", registry=registry)
metric_query_duration_us = Counter("QueryDuratioin", "metric_query_duration_us", registry=registry)
metric_inf_duration_us = Counter("InferDuration", "metric_inf_duration_us", registry=registry)
