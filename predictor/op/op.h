#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_OP_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_OP_H

#include <bvar/bvar.h> // bvar::LatencyRecorder
#include "common/inner_common.h"
#include "framework/channel.h"
#include "framework/op_repository.h"
#include "framework/predictor_metric.h" // PredictorMetric

namespace baidu {
namespace paddle_serving {
namespace predictor {

class Dag;

class Op {
public:
    Op() : _bus(NULL), 
        _dag(NULL), 
        _has_calc(false), 
        _has_init(false), 
        _timer(NULL) {}

    virtual ~Op() {}

    // ------对依赖OP的Channel/Data/Message数据获取接口-----

    // 获得依赖Op的Channel对象
    Channel* mutable_depend_channel(const std::string& op);

    // 获得依赖Op的Channel对象
    const Channel* get_depend_channel(const std::string& op) const;

    template<typename T>
    T* mutable_depend_argument(const std::string& op) {
        Channel* channel = mutable_depend_channel(op); 
        if (channel == NULL) {
            LOG(WARNING) << "cannot mutable channel of " << op 
                    << " in " << _name;
            return NULL;
        }

        OpChannel<T>* op_channel = 
            dynamic_cast<OpChannel<T>*>(channel);
        if (!op_channel) {
            LOG(FATAL) << "Cannot dynamic cast channel of op:" 
                << this->name() << " to type: " << typeid(T).name();
            return NULL;
        }

        return op_channel->data();
    }

    template<typename T>
    const T* get_depend_argument(const std::string& op) const {
        const Channel* channel = get_depend_channel(op);
        if (channel == NULL) {
            LOG(WARNING) << "cannot get read-only channel of " << op 
                    << " in " << _name;
            return NULL;
        }

        const OpChannel<T>* op_channel = 
            dynamic_cast<const OpChannel<T>*>(channel);
        if (!op_channel) {
            LOG(FATAL) << "Cannot dynamic cast channel of op:" 
                << this->name() << " to type: " << typeid(T).name();
            return NULL;
        }

        return op_channel->data();
    }

    // -----对当前OP的Channel/Data/Message数据获取接口----

    // 获得该OP的Protobuf message类型指针
    google::protobuf::Message* mutable_message();

    // 获得该OP的Protobuf message类型指针
    const google::protobuf::Message* get_message() const;

    // 获得该OP的模板类数据对象
    template<typename T> 
    T* mutable_data() {
        Channel* channel = mutable_channel();
        return (dynamic_cast<OpChannel<T>*>(channel))->data();
    }

    // 获得该OP的模板类数据对象
    template<typename T> 
    const T* get_data() const {
        const Channel* channel = get_channel();
        return (dynamic_cast<const OpChannel<T>*>(channel))->data(); 
    }

    // ---------------- 其它基类成员函数 ----------------

    int init(Bus* bus, Dag* dag, uint32_t id, const std::string& name,
            const std::string& type, void* conf);

    int deinit();

    int check_time(const char* tag);

    int process(bool debug);

    std::string time_info();

    // 获得输入对象
    const google::protobuf::Message* get_request_message();

    bool has_calc();

    const char* name() const;

    const std::string& full_name() const {
        return _full_name;
    }
    
    void set_full_name(const std::string full_name) {
        _full_name = full_name;
    }
    
    const std::string& type() const;

    uint32_t id() const;

    // --------------- Default implements ----------------

    virtual int custom_init() { return 0; }

    virtual int custom_deinit() { return 0; }

    virtual const std::string debug_string();

    // ------------------ OP Interface -------------------

    // 获得当前Op的Channel派生类对象
    virtual Channel* mutable_channel() = 0;

    // 获得当前Op的Channel派生类对象
    virtual const Channel* get_channel() const = 0;

    // 释放当前Op的Channel派生类对象
    virtual int release_channel() = 0;

    // 当前Op自定义inference函数接口
    virtual int inference() = 0;

    // ------------------ Conf Interface -------------------
    virtual void* create_config(const comcfg::ConfigUnit& conf) { return NULL; }
    
    virtual void delete_config(void* conf) { }

    virtual void set_config(void* conf) { return; }

    // ------------------ Metric Interface -------------------
    virtual void regist_metric() { return; }
     
private:
    bool is_mutable(const std::string& op);

    bool is_mutable(const std::string& op) const;

    bool is_readable(const std::string& op);

    bool is_readable(const std::string& op) const;

private:
    Bus* _bus;
    Dag* _dag;
    uint32_t _id;
    std::string _name;
    std::string _full_name; // service_workflow_stageindex_opname 
    std::string _type;
    bool _has_calc;
    bool _has_init;
    TimerFlow* _timer;
};

template<typename T>
class OpWithChannel : public Op {
public:
    typedef T DataType;
    typedef OpChannel<T> ChannelType;

    OpWithChannel() : _channel(NULL) {}

    virtual ~OpWithChannel() {}

    // ---------- Implements ----------

    Channel* mutable_channel() {
        if (_channel != NULL) {
            return _channel;
        } 

        _channel = base::get_object<ChannelType>();
        if (!_channel) {
            LOG(FATAL) 
                << "Failed mutable channel of type:"
                << typeid(T).name();
            return NULL;
        }
        _channel->init(this->id(), this->name());
        return _channel;
    }

    const Channel* get_channel() const {
        return _channel;
    }

    int release_channel() {
        if (_channel) {
            _channel->deinit();
            base::return_object<ChannelType>(_channel);
        } 

        _channel = NULL; 
        return 0;
    }

    // ------------- Interface -------------

    // Op自定义inference接口
    virtual int inference() = 0;

private:
    ChannelType* _channel;
};

template<typename T, typename C>
class OpWithChannelAndConf : public OpWithChannel<T> {
public:
    void set_config(void* conf) {
        _conf = static_cast<C*>(conf);
    }

    C* get_self_config() { return _conf; }

    virtual void delete_config(void* conf) { delete static_cast<C*>(conf); }

private:
    C* _conf;
};

#define DECLARE_OP(OP_TYPE)         \
    OP_TYPE() {                     \
        REGISTER_OP(OP_TYPE);       \
    }                               \
    static OP_TYPE _s_##OP_TYPE     \

#define DEFINE_OP(OP_TYPE)          \
    OP_TYPE OP_TYPE::_s_##OP_TYPE   \

} // predictor
} // paddle_serving
} // baidu

#endif
