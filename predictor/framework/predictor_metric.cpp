#include "predictor_metric.h"
#include "butil/memory/singleton.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

PredictorMetric* PredictorMetric::GetInstance() {
    return Singleton<PredictorMetric>::get();
}

} // namespace predictor
} // namespace paddle_serving 
} // namespace baidu
