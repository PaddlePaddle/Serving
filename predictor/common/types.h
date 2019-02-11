#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_TYPES_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_TYPES_H

namespace baidu {
namespace paddle_serving {
namespace predictor {

typedef char* Byte;
typedef size_t Size;
typedef const char* ConstByte;

struct Sequence {
    Byte data;
    Size size;
};

} // predictor
} // paddle_serving
} // baidu

#endif // BAIDU_PADDLE_SERVING_PREDICTOR_TYPES_H
