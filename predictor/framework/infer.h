#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_INFER_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_INFER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "common/inner_common.h"
#include "framework/infer_data.h"
#include "framework/factory.h"
#include "framework/bsf.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class InferEngine {
public:

    virtual ~InferEngine() {}

    virtual int proc_initialize(const comcfg::ConfigUnit& conf, bool version) {
        return proc_initialize_impl(conf, version);
    }
    virtual int proc_finalize() {
        return proc_finalize_impl();
    }
    virtual int thrd_initialize() {
        return thrd_initialize_impl();
    }
    virtual int thrd_clear() {
        return thrd_clear_impl();
    }
    virtual int thrd_finalize() {
        return thrd_finalize_impl();
    }
    virtual int infer(const void* in, void* out, uint32_t batch_size = -1) {
        return infer_impl1(in, out, batch_size);
    }

    virtual int reload() = 0;

    virtual uint64_t version() const = 0;

    // begin: framework inner call
    virtual int proc_initialize_impl(
            const comcfg::ConfigUnit& conf, bool version) = 0;
    virtual int thrd_initialize_impl() = 0;
    virtual int thrd_finalize_impl() = 0;
    virtual int thrd_clear_impl() = 0;
    virtual int proc_finalize_impl() = 0;
    virtual int infer_impl1(
            const void* in, void* out, uint32_t batch_size = -1) = 0;
    virtual int infer_impl2(const BatchTensor& in, BatchTensor& out) = 0;
    // end: framework inner call
};

class ReloadableInferEngine : public InferEngine {
public:
    virtual ~ReloadableInferEngine() {}

    union last_check_status {
        time_t last_timestamp;
        uint64_t last_md5sum;
        uint64_t last_revision;
    };

    typedef im::bsf::Task<Tensor, Tensor> TaskT;

    virtual int load(const std::string& data_path) = 0;

    int proc_initialize_impl(const comcfg::ConfigUnit& conf, bool version) {
        _reload_tag_file = conf["ReloadableMeta"].to_cstr();
        _reload_mode_tag = conf["ReloadableType"].to_cstr();
        _model_data_path = conf["ModelDataPath"].to_cstr();
        _infer_thread_num = conf["RuntimeThreadNum"].to_uint32();
        _infer_batch_size = conf["BatchInferSize"].to_uint32();
        _infer_batch_align = conf["EnableBatchAlign"].to_uint32();
        if (!check_need_reload() || load(_model_data_path) != 0) {
            LOG(FATAL) << "Failed load model_data_path" << _model_data_path;
            return -1;
        }

        if (parse_version_info(conf, version) != 0) {
            LOG(FATAL) << "Failed parse version info";
            return -1;
        }

        LOG(WARNING) << "Succ load model_data_path" << _model_data_path;
        return 0;
    }

    int proc_initialize(const comcfg::ConfigUnit& conf, bool version) {
        if (proc_initialize_impl(conf, version) != 0) {
            LOG(FATAL) << "Failed proc initialize impl";
            return -1;
        }

        // init bsf framework
        if (_infer_thread_num <= 0) {
            return 0;
        }

        im::bsf::TaskExecutor<TaskT>::instance()->set_thread_init_fn(
                boost::bind(&InferEngine::thrd_initialize_impl, this));
        im::bsf::TaskExecutor<TaskT>::instance()->set_thread_reset_fn(
                boost::bind(&InferEngine::thrd_clear_impl, this));
        im::bsf::TaskExecutor<TaskT>::instance()->set_thread_callback_fn(
                boost::bind(&InferEngine::infer_impl2, this, _1, _2));
        im::bsf::TaskExecutor<TaskT>::instance()->set_batch_size(_infer_batch_size);
        im::bsf::TaskExecutor<TaskT>::instance()->set_batch_align(_infer_batch_align);
        if (im::bsf::TaskExecutor<TaskT>::instance()->start(_infer_thread_num)
                != 0) {
            LOG(FATAL) << "Failed start bsf executor, threads:" << _infer_thread_num;
            return -1;
        }

        LOG(WARNING) << "Enable batch schedule framework, thread_num:"
            << _infer_thread_num << ", batch_size:" << _infer_batch_size
            << ", enable_batch_align:" << _infer_batch_align;

        return 0;
    }

    int infer(const void* in, void* out, uint32_t batch_size = -1) {
        if (_infer_thread_num <= 0) {
            return infer_impl1(in, out, batch_size);
        }

        im::bsf::TaskManager<Tensor, Tensor> task_manager;
        task_manager.schedule(*(const BatchTensor*)in, *(BatchTensor*)out);
        task_manager.wait();
        return 0;
    }

    int thrd_initialize() {
        if (_infer_thread_num > 0) {
            return 0;
        }

        return thrd_initialize_impl();
    }

    int thrd_clear() {
        if (_infer_thread_num > 0) {
            return 0;
        }

        return thrd_clear_impl();
    }

    int proc_finalize() {
        if (proc_finalize_impl() != 0) {
            LOG(FATAL) << "Failed proc finalize impl";
            return -1;
        }

        if (_infer_thread_num > 0) {
            im::bsf::TaskExecutor<TaskT>::instance()->stop();
        }

        return 0;
    }

    int reload() {
        if (check_need_reload()) {
            LOG(WARNING) << "begin reload model[" << _model_data_path << "].";
            return load(_model_data_path);
        }
        return 0;
    }

    uint64_t version() const {
        return _version; 
    }

    uint32_t thread_num() const {
        return _infer_thread_num;
    }

private:
    int parse_version_info(const comcfg::ConfigUnit& config, bool version) {
        try {
            std::string version_file = config["VersionFile"].to_cstr();
            std::string version_type = config["VersionType"].to_cstr();
            
            if (version_type == "abacus_version") {
                if (parse_abacus_version(version_file) != 0) {
                    LOG(FATAL) 
                        << "Failed parse abacus version: " << version_file;
                    return -1;
                }
            } else if (version_type == "corece_uint64") {
                if (parse_corece_uint64(version_file) != 0) {
                    LOG(FATAL) 
                        << "Failed parse corece_uint64: " << version_file;
                    return -1;
                }
            } else {
                LOG(FATAL) << "Not supported version_type: " << version_type;
                return -1;
            }
        } catch (comcfg::ConfigException e) { // no version file
            if (version) {
                LOG(FATAL) << "Cannot parse version engine, err:" 
                    << e.what();
                return -1;
            }

            LOG(WARNING) << "Consistency with non-versioned configure";
            _version = uint64_t(-1);
        }
        return 0;
    }

    int parse_abacus_version(const std::string& version_file) {
        FILE* fp = fopen(version_file.c_str(), "r");
        if (!fp) {
            LOG(FATAL) << "Failed open version file:" << version_file; 
            return -1;
        }

        bool has_parsed = false;
        char buffer[1024] = {0};
        while (fgets(buffer, sizeof(buffer), fp)) {
            char* begin = NULL;
            if (strncmp(buffer, "version:", 8) == 0 ||
                    strncmp(buffer, "Version:", 8) == 0) {
                begin = buffer + 8;
            } else if (strncmp(buffer, "version :", 9) == 0 || 
                    strncmp(buffer, "Version :", 9) == 0) {
                begin = buffer + 9;
            } else {
                LOG(WARNING) << "Not version line: " << buffer; 
                continue;
            }

            std::string vstr = begin;
            boost::algorithm::trim_if(
                    vstr, boost::algorithm::is_any_of("\n\r "));
            char* endptr = NULL;
            _version = strtoull(vstr.c_str(), &endptr, 10);
            if (endptr == vstr.c_str()) {
                LOG(FATAL) 
                    << "Invalid version: [" << buffer << "], end: [" 
                    << endptr << "]" << ", vstr: [" << vstr << "]";
                fclose(fp);
                return -1;
            }
            has_parsed = true;
        }

        if (!has_parsed) {
            LOG(FATAL) << "Failed parse abacus version: " << version_file; 
            fclose(fp);
            return -1;
        }

        LOG(WARNING) << "Succ parse abacus version: " << _version 
            << " from: " << version_file;
        fclose(fp);
        return 0;
    }

    int parse_corece_uint64(const std::string& version_file) {
        FILE* fp = fopen(version_file.c_str(), "r");
        if (!fp) {
            LOG(FATAL) << "Failed open version file:" << version_file; 
            return -1;
        }

        bool has_parsed = false;
        char buffer[1024] = {0};
        if (fgets(buffer, sizeof(buffer), fp)) {
            char* endptr = NULL;
            _version = strtoull(buffer, &endptr, 10);
            if (endptr == buffer) {
                LOG(FATAL) << "Invalid version: " << buffer;
                fclose(fp);
                return -1;
            }
            has_parsed = true;
        }

        if (!has_parsed) {
            LOG(FATAL) << "Failed parse abacus version: " << version_file; 
            fclose(fp);
            return -1;
        }

        LOG(WARNING) << "Succ parse corece version: " << _version 
            << " from: " << version_file;
        fclose(fp);
        return 0; 
    }

    bool check_need_reload() {
        if (_reload_mode_tag == "timestamp_ne") {
            return check_timestamp_ne();
        } else if (_reload_mode_tag == "timestamp_gt") {
            return check_timestamp_gt();
        } else if (_reload_mode_tag == "md5sum") {
            return check_md5sum();
        } else if (_reload_mode_tag == "revision") {
            return check_revision();
        } else if (_reload_mode_tag == "none") {
            return false;
        } else {
            LOG(FATAL) << "Not support check type: "
                << _reload_mode_tag;
            return false;
        }
    }

    bool check_timestamp_ne() {
        struct stat st;
        if (stat(_reload_tag_file.c_str(), &st) != 0) {
            LOG(FATAL) << "Failed stat config file:"
                << _reload_tag_file;
            return false;
        }

        if ((st.st_mode & S_IFREG) &&
                st.st_mtime != _last_status.last_timestamp) {
            _last_status.last_timestamp = st.st_mtime;
            return true;
        }

        return false;
    }

    bool check_timestamp_gt() {
        struct stat st;
        if (stat(_reload_tag_file.c_str(), &st) != 0) {
            LOG(FATAL) << "Failed stat config file:"
                << _reload_tag_file;
            return false;
        }

        if ((st.st_mode & S_IFREG) &&
                st.st_mtime > _last_status.last_timestamp) {
            _last_status.last_timestamp = st.st_mtime;
            return true;
        }

        return false;
    }

    bool check_md5sum() {
        return false;
    }

    bool check_revision() {
        return false;
    }

protected:
    std::string _model_data_path;

private:
    std::string _reload_tag_file;
    std::string _reload_mode_tag;
    last_check_status _last_status;
    uint32_t _infer_thread_num;
    uint32_t _infer_batch_size;
    bool _infer_batch_align;
    uint64_t _version;
};

template<typename EngineCore>
struct ModelData {
    ModelData() : current_idx(1) {
        cores[0] = NULL;
        cores[1] = NULL;
    }

    ~ModelData() {
        delete cores[0];
        delete cores[1];
    }

    EngineCore* cores[2];
    uint32_t current_idx;
}; 

template<typename EngineCore>
class DBReloadableInferEngine : public ReloadableInferEngine {
public:
    virtual ~DBReloadableInferEngine() {}

    int proc_initialize(const comcfg::ConfigUnit& conf, bool version) {
        THREAD_KEY_CREATE(&_skey, NULL);
        THREAD_MUTEX_INIT(&_mutex, NULL);
        return ReloadableInferEngine::proc_initialize(conf, version);
    }

    virtual int load(const std::string& model_data_dir) {
        if (_reload_vec.empty()) {
            return 0;
        }

        for (uint32_t ti = 0; ti < _reload_vec.size(); ++ti) {
            if (load_data(_reload_vec[ti], model_data_dir) != 0) {
                LOG(FATAL) << "Failed reload engine model: " << ti;
                return -1;
            }
        }
        
        LOG(WARNING) << "Succ load engine, path: " << model_data_dir;

        return 0;
    }

    int load_data(ModelData<EngineCore>* md, const std::string& data_path) {
        uint32_t next_idx = (md->current_idx + 1) % 2;
        if (md->cores[next_idx]) {
            delete md->cores[next_idx];
        }

        md->cores[next_idx] = new (std::nothrow) EngineCore;
        if (!md->cores[next_idx]
                || md->cores[next_idx]->create(data_path) != 0) {
            LOG(FATAL) << "Failed create model, path: " << data_path;
            return -1;
        }
        md->current_idx = next_idx;
        return 0;
    }

    virtual int thrd_initialize_impl() {
        // memory pool to be inited in non-serving-threads
        if (MempoolWrapper::instance().thread_initialize() != 0) {
            LOG(FATAL) << "Failed thread initialize mempool";
            return -1;
        }
        
        ModelData<EngineCore>* md = new(std::nothrow) ModelData<EngineCore>;
        if (!md || load_data(md, _model_data_path) != 0) {
            LOG(FATAL) << "Failed create thread data from " << _model_data_path;
            return -1;
        }

        THREAD_SETSPECIFIC(_skey, md);
        im::bsf::AutoMutex lock(_mutex);
        _reload_vec.push_back(md);
        return 0;
    }

    int thrd_clear_impl() {
        // for non-serving-threads
        if (MempoolWrapper::instance().thread_clear() != 0) {
            LOG(FATAL) << "Failed thread clear mempool";
            return -1;
        }
        return 0;
    }

    int thrd_finalize_impl() {
        return 0;
    }

    int proc_finalize_impl() {
        THREAD_KEY_DELETE(_skey);
        THREAD_MUTEX_DESTROY(&_mutex);
        return 0;
    }

    EngineCore* get_core() {
        ModelData<EngineCore>* md = (ModelData<EngineCore>*)THREAD_GETSPECIFIC(_skey);
        if (!md) {
            LOG(FATAL) << "Failed get thread specific data";
            return NULL;
        }
        return md->cores[md->current_idx];
    }

protected:
    THREAD_KEY_T _skey;
    THREAD_MUTEX_T _mutex;
    std::vector<ModelData<EngineCore>*> _reload_vec;
private:
};

// 多个EngineCore共用同一份模型数据
template<typename EngineCore>
class CloneDBReloadableInferEngine : public DBReloadableInferEngine<EngineCore> {
public:
    virtual ~CloneDBReloadableInferEngine() {}

    virtual int proc_initialize(const comcfg::ConfigUnit& conf, bool version) {
        _pd = new (std::nothrow) ModelData<EngineCore>;
        if (!_pd) {
            LOG(FATAL) << "Failed to allocate for ProcData";
            return -1;
        }
        return DBReloadableInferEngine<EngineCore>::proc_initialize(
                conf, version);
    }

    virtual int load(const std::string& model_data_dir) {
        // 加载进程级模型数据
        if (!_pd || DBReloadableInferEngine<EngineCore>::load_data(
                    _pd, model_data_dir) != 0) {
            LOG(FATAL) 
                << "Failed to create common model from [" 
                << model_data_dir << "].";
            return -1;
        }
        LOG(WARNING) 
            << "Succ load common model[" << _pd->cores[_pd->current_idx] 
            << "], path[" << model_data_dir << "].";

        if (DBReloadableInferEngine<EngineCore>::_reload_vec.empty()) {
            return 0;
        }

        for (uint32_t ti = 0; ti < DBReloadableInferEngine<EngineCore>::_reload_vec.size(); ++ti) {
            if (load_data(DBReloadableInferEngine<EngineCore>::_reload_vec[ti],
                        _pd->cores[_pd->current_idx]) != 0) {
                LOG(FATAL) << "Failed reload engine model: " << ti;
                return -1;
            }
        }
        
        LOG(WARNING) << "Succ load clone model, path[" << model_data_dir << "]";

        return 0;
    }

    // 加载线程级对象，多个线程级对象共用pd_core的模型数据
    int load_data(
            ModelData<EngineCore>* td,
            EngineCore* pd_core) {
        uint32_t next_idx = (td->current_idx + 1) % 2;
        if (td->cores[next_idx]) {
            delete td->cores[next_idx];
        }

        td->cores[next_idx] = new (std::nothrow) EngineCore;
        if (!td->cores[next_idx]
                || td->cores[next_idx]->clone(pd_core->get()) != 0) {
            LOG(FATAL) << "Failed clone model from pd_core[ " << pd_core
                    << "], idx[" << next_idx << "]";
            return -1;
        }
        td->current_idx = next_idx;
        LOG(WARNING) 
            << "td_core[" << td->cores[td->current_idx] 
            << "] clone model from pd_core["
            << pd_core << "] succ, cur_idx[" << td->current_idx << "].";
        return 0;
    }

    virtual int thrd_initialize_impl() {
        // memory pool to be inited in non-serving-threads
        if (MempoolWrapper::instance().thread_initialize() != 0) {
            LOG(FATAL) << "Failed thread initialize mempool";
            return -1;
        }
        
        ModelData<EngineCore>* md = new(std::nothrow) ModelData<EngineCore>;
        if (!md || load_data(md, _pd->cores[_pd->current_idx]) != 0) {
            LOG(FATAL) << "Failed clone thread data, origin_core["
                    << _pd->cores[_pd->current_idx] << "].";
            return -1;
        }

        THREAD_SETSPECIFIC(DBReloadableInferEngine<EngineCore>::_skey, md);
        im::bsf::AutoMutex lock(DBReloadableInferEngine<EngineCore>::_mutex);
        DBReloadableInferEngine<EngineCore>::_reload_vec.push_back(md);
        return 0;
    }

protected:
    ModelData<EngineCore>* _pd; // 进程级EngineCore，多个线程级EngineCore共用该对象的模型数据
};

template<typename FluidFamilyCore>
class FluidInferEngine : public DBReloadableInferEngine<FluidFamilyCore> {
public:
    FluidInferEngine() {}
    ~FluidInferEngine() {}
    
    int infer_impl1(const void* in, void* out, uint32_t batch_size = -1) {
        FluidFamilyCore* core
            = DBReloadableInferEngine<FluidFamilyCore>::get_core();
        if (!core || !core->get()) {
            LOG(FATAL) << "Failed get fluid core in infer_impl()";
            return -1;
        }
        
        if (!core->Run(in, out)) {
            LOG(FATAL) << "Failed run fluid family core";
            return -1;
        }
        return 0;
    }

    int infer_impl2(const BatchTensor& in, BatchTensor& out) {
        return infer_impl1(&in, &out);
    }
};

template<typename TensorrtFamilyCore>
class TensorrtInferEngine : public DBReloadableInferEngine<TensorrtFamilyCore> {
public:
    TensorrtInferEngine() {}
    ~TensorrtInferEngine() {}

    int infer_impl1(const void* in, void* out, uint32_t batch_size) {
        TensorrtFamilyCore* core
            = DBReloadableInferEngine<TensorrtFamilyCore>::get_core();
        if (!core || !core->get()) {
            LOG(FATAL) << "Failed get fluid core in infer_impl()";
            return -1;
        }

        if (!core->Run(in, out, batch_size)) {
            LOG(FATAL) << "Failed run fluid family core";
            return -1;
        }
        return 0;
    }

    int infer_impl2(const BatchTensor& in, BatchTensor& out) {
        LOG(FATAL) << "Tensortrt donot supports infer_impl2 yet!";
        return -1;
    }
};

template<typename AbacusFamilyCore>
class AbacusInferEngine : public CloneDBReloadableInferEngine<AbacusFamilyCore> {
public:
    AbacusInferEngine() {}
    ~AbacusInferEngine() {}

    int infer_impl1(const void* in, void* out, uint32_t batch_size = -1) {
        LOG(FATAL) << "Abacus dnn engine must use predict interface";
        return -1;
    }

    int infer_impl2(const BatchTensor& in, BatchTensor& out) {
        LOG(FATAL) << "Abacus dnn engine must use predict interface";
        return -1;
    }

    // Abacus special interface
    int predict(uint32_t ins_num) {
        AbacusFamilyCore* core
                = CloneDBReloadableInferEngine<AbacusFamilyCore>::get_core();
        if (!core || !core->get()) {
            LOG(FATAL) << "Failed get abacus core in predict()";
            return -1;
        }

        return core->predict(ins_num);
    }
    int set_use_fpga(bool use_fpga) {
        AbacusFamilyCore* core
                = CloneDBReloadableInferEngine<AbacusFamilyCore>::get_core();
        if (!core || !core->get()) {
            LOG(FATAL) << "Failed get abacus core in predict()";
            return -1;
        }

        return core->set_use_fpga(use_fpga);
    }
    int debug() {
        AbacusFamilyCore* core
                = CloneDBReloadableInferEngine<AbacusFamilyCore>::get_core();
        if (!core || !core->get()) {
            LOG(FATAL) << "Failed get abacus core in debug()";
            return -1;
        }
        return core->debug();
    }

    int set_search_id(uint64_t sid) {
        AbacusFamilyCore* core
                = CloneDBReloadableInferEngine<AbacusFamilyCore>::get_core();
        if (!core || !core->get()) {
            LOG(FATAL) << "Failed get abacus core in set_serach_id()";
            return -1;
        }
        return core->set_search_id(sid);
    }

    int set_hidden_layer_dim(uint32_t dim) {
        AbacusFamilyCore* core
                = CloneDBReloadableInferEngine<AbacusFamilyCore>::get_core();
        if (!core || !core->get()) {
            LOG(FATAL) << "Failed get abacus core in set_layer_dim()";
            return -1;
        }
        return core->set_hidden_layer_dim(dim);
    }

    int get_input(
            uint32_t ins_idx, uint32_t* fea_num, void* in) {
        AbacusFamilyCore* core
                = CloneDBReloadableInferEngine<AbacusFamilyCore>::get_core();
        if (!core || !core->get()) {
            LOG(FATAL) << "Failed get abacus core in get_input()";
            return -1;
        }
        return core->get_input(ins_idx, fea_num, in);
    }

    int get_layer_value(const std::string& name,
            uint32_t ins_num, uint32_t fea_dim, void* out) {
        AbacusFamilyCore* core
                = CloneDBReloadableInferEngine<AbacusFamilyCore>::get_core();
        if (!core || !core->get()) {
            LOG(FATAL) << "Failed get abacus core in get_layer_value()";
            return -1;
        }
        return core->get_layer_value(name, ins_num, fea_dim, out);
    }
    
    void set_position_idx(void* input, uint64_t fea, uint32_t ins_idx) {
        AbacusFamilyCore* core
                = CloneDBReloadableInferEngine<AbacusFamilyCore>::get_core();
        if (!core || !core->get()) {
            LOG(FATAL) << "Failed get abacus core in set_position_idx()";
            return;
        }
        core->set_position_idx(input, fea, ins_idx);
        return;
    }
};

template<typename PaddleV2FamilyCore>
class PaddleV2InferEngine : public CloneDBReloadableInferEngine<PaddleV2FamilyCore> {
public:
    PaddleV2InferEngine() {}
    ~PaddleV2InferEngine() {}
    
    int infer_impl1(const void* in, void* out, uint32_t batch_size = -1) {
        LOG(FATAL) << "Paddle V2 engine must use predict interface";
        return -1;
    }

    int infer_impl2(const BatchTensor& in, BatchTensor& out) {
        LOG(FATAL) << "Paddle V2 engine must use predict interface";
        return -1;
    }
};

typedef FactoryPool<InferEngine> StaticInferFactory;

class VersionedInferEngine : public InferEngine {
public:
    VersionedInferEngine() {
        _versions.clear();
    }
    ~VersionedInferEngine() {}

    int proc_initialize(const comcfg::ConfigUnit& conf) {
        size_t version_num = conf["Version"].size();
        for (size_t vi = 0; vi < version_num; ++vi) {
            if (proc_initialize(conf["Version"][vi], true) != 0) {
                LOG(FATAL) << "Failed proc initialize version: " 
                    << vi << ", model: " << conf["Name"].to_cstr();
                return -1;
            }
        }

        if (version_num == 0) {
            if (proc_initialize(conf, false) != 0) {
                LOG(FATAL) << "Failed proc intialize engine: " 
                    << conf["Name"].to_cstr();
                return -1;
            }
        }
        LOG(WARNING) 
            << "Succ proc initialize engine: " << conf["Name"].to_cstr();
        return 0;
    }

    int proc_initialize(const comcfg::ConfigUnit& conf, bool version) {
        std::string engine_type = conf["Type"].to_cstr();
        InferEngine* engine
            = StaticInferFactory::instance().generate_object(
                    engine_type);
        if (!engine) {
            LOG(FATAL) << "Failed generate engine with type:"
                << engine_type;
            return -1;
        }

        if (engine->proc_initialize(conf, version) != 0) {
            LOG(FATAL) << "Failed initialize engine, type:"
                << engine_type;
            return -1;
        }

        auto r = _versions.insert(std::make_pair(engine->version(), engine));
        if (!r.second) {
            LOG(FATAL) << "Failed insert item: " << engine->version() 
                << ", type: " << engine_type;
            return -1;
        }
        LOG(WARNING) 
            << "Succ proc initialize version engine: " << engine->version();
        return 0;
    }

    int proc_finalize() {
        for (auto iter = _versions.begin(); iter != _versions.end(); ++iter) {
            if (iter->second->proc_finalize() != 0) {
                LOG(FATAL) << "Failed proc finalize version engine: " <<
                    iter->first;
            }
            LOG(WARNING) 
                << "Succ proc finalize version engine: " << iter->first;
        }
        return 0;
    }

    int thrd_initialize() {
        for (auto iter = _versions.begin(); iter != _versions.end(); ++iter) {
            if (iter->second->thrd_initialize() != 0) {
                LOG(FATAL) << "Failed thrd initialize version engine: " <<
                    iter->first;
                return -1;
            }
            LOG(WARNING) 
                << "Succ thrd initialize version engine: " << iter->first;
        }
        return 0;
    }

    int thrd_clear() {
        for (auto iter = _versions.begin(); iter != _versions.end(); ++iter) {
            if (iter->second->thrd_clear() != 0) {
                LOG(FATAL) << "Failed thrd clear version engine: " << 
                    iter->first;
                return -1;
            }
            LOG(INFO) << "Succ thrd clear version engine: " << iter->first;
        }
        return 0;
    }

    int thrd_finalize() {
        for (auto iter = _versions.begin(); iter != _versions.end(); ++iter) {
            if (iter->second->thrd_finalize() != 0) {
                LOG(FATAL) << "Failed thrd finalize version engine: " << 
                    iter->first;
                return -1;
            }
            LOG(WARNING) << "Succ thrd finalize version engine: " << iter->first;
        }
        return 0;
    }

    int reload() {
        for (auto iter = _versions.begin(); iter != _versions.end(); ++iter) {
            if (iter->second->reload() != 0) {
                LOG(FATAL) << "Failed reload version engine: " << 
                    iter->first;
                return -1;
            }
            LOG(WARNING) << "Succ reload version engine: " << iter->first;
        }
        return 0; 
    }

    uint64_t version() const {
        InferEngine* engine = default_engine();
        if (engine) {
            return engine->version();
        } else {
            return uint64_t(-1);
        }
    }

    // inference interface
    InferEngine* default_engine() const {
        if (_versions.size() != 1) {
            LOG(FATAL) << "Ambiguous default engine version:" 
                << _versions.size();
            return NULL;
        }

        return _versions.begin()->second;
    }
    
    int infer(const void* in, void* out, uint32_t batch_size) {
        InferEngine* engine = default_engine();
        if (!engine) {
            LOG(WARNING) << "fail to get default engine";
            return -1;
        }
        return engine->infer(in, out, batch_size);
    }

    template<typename T>
    T* get_core() {
        InferEngine* engine = default_engine();
        if (!engine) {
            LOG(WARNING) << "fail to get core";
            return NULL;
        }
        auto db_engine = dynamic_cast<DBReloadableInferEngine<T>*>(engine);
        if (db_engine) {
            return db_engine->get_core();
        }
        LOG(WARNING) << "fail to get core";
        return NULL;
    }

    // versioned inference interface
    int infer(
            const void* in, void* out, uint32_t batch_size, uint64_t version) {
        auto iter = _versions.find(version);
        if (iter == _versions.end()) {
            LOG(FATAL) << "Not found version engine: " << version;
            return -1;
        }

        return iter->second->infer(in, out, batch_size);
    }
    
    template<typename T>
    T* get_core(uint64_t version) {
        auto iter = _versions.find(version);
        if (iter == _versions.end()) {
            LOG(FATAL) << "Not found version engine: "  << version;
            return NULL;
        }

        auto db_engine = dynamic_cast<DBReloadableInferEngine<T>*>(iter->second);
        if (db_engine) {
            return db_engine->get_core();
        }
        LOG(WARNING) << "fail to get core for " << version;
        return NULL;
    }

    // --
    int proc_initialize_impl(const comcfg::ConfigUnit& conf, bool) { return -1; }
    int thrd_initialize_impl() { return -1; }
    int thrd_finalize_impl() { return -1; }
    int thrd_clear_impl() { return -1; }
    int proc_finalize_impl() { return -1; }
    int infer_impl1(const void* in, void* out, uint32_t batch_size = -1) { return -1; }
    int infer_impl2(const BatchTensor& in, BatchTensor& out) { return -1; }

private:
    boost::unordered_map<uint64_t, InferEngine*> _versions;
};

class InferManager {
public:
    static InferManager& instance() {
        static InferManager ins;
        return ins;
    }

    int proc_initialize(const char* path, const char* file) {
        comcfg::Configure conf;
        if (conf.load(path, file) != 0) {
            LOG(FATAL) << "failed load infer config, path:"
                << path << "/" << file;
            return -1;
        }

        size_t engine_num = conf["Engine"].size();
        for (size_t ei = 0; ei < engine_num; ++ei) {
            std::string engine_name = conf["Engine"][ei]["Name"].to_cstr();
            VersionedInferEngine* engine = new (std::nothrow) VersionedInferEngine();
            if (!engine) {
                LOG(FATAL) << "Failed generate versioned engine: " << engine_name;
                return -1;
            }

            if (engine->proc_initialize(conf["Engine"][ei]) != 0) {
                LOG(FATAL) << "Failed initialize version engine, name:"
                    << engine_name;
                return -1;
            }

            auto r = _map.insert(std::make_pair(engine_name, engine));
            if (!r.second) {
                LOG(FATAL) << "Failed insert item: " << engine_name;
                return -1;
            }
            LOG(WARNING) << "Succ proc initialize engine: " << engine_name;
        }

        return 0;
    }

    int thrd_initialize() {
        for (auto it = _map.begin(); it != _map.end(); ++it) {
            if (it->second->thrd_initialize() != 0) {
                LOG(FATAL) << "Failed thrd initialize engine, name: "
                    << it->first;
                return -1;
            }
            LOG(WARNING) << "Succ thrd initialize engine, name: "
                << it->first;
        }
        return 0;
    }

    int thrd_clear() {
        for (auto it = _map.begin(); it != _map.end(); ++it) {
            if (it->second->thrd_clear() != 0) {
                LOG(FATAL) << "Failed thrd clear engine, name: "
                    << it->first;
                return -1;
            }
        }
        return 0;
    }

    int reload() {
        for (auto it = _map.begin(); it != _map.end(); ++it) {
            if (it->second->reload() != 0) {
                LOG(FATAL) << "Failed reload engine, name: "
                    << it->first;
                return -1;
            }
        }
        return 0;
    }

    int thrd_finalize() {
        for (auto it = _map.begin(); it != _map.end(); ++it) {
            if (it->second->thrd_finalize() != 0) {
                LOG(FATAL) << "Failed thrd finalize engine, name: "
                    << it->first;
                return -1;
            }
            LOG(WARNING) << "Succ thrd finalize engine, name: "
                << it->first;
        }
        return 0;
    }

    int proc_finalize() {
        for (auto it = _map.begin(); it != _map.end(); ++it) {
            if (it->second->proc_finalize() != 0) {
                LOG(FATAL) << "Failed proc finalize engine, name: "
                    << it->first;
                return -1;
            }
            LOG(WARNING) << "Succ proc finalize engine, name: "
                << it->first;
        }
        return 0;
    }

    // Inference interface
    int infer(const char* model_name, const void* in, void* out, uint32_t batch_size = -1) {
        auto it = _map.find(model_name);
        if (it == _map.end()) {
            LOG(WARNING) << "Cannot find engine in map, model name:"
                << model_name;
            return -1;
        }
        return it->second->infer(in, out, batch_size);
    }

    template<typename T>
    T* get_core(const char* model_name) {
        auto it = _map.find(model_name);
        if (it == _map.end()) {
            LOG(WARNING) << "Cannot find engine in map, model name:"
                << model_name;
            return NULL;
        }
        auto infer_engine = dynamic_cast<DBReloadableInferEngine<T>*>(
                it->second->default_engine());
        if (infer_engine) {
            return infer_engine->get_core();
        }
        LOG(WARNING) << "fail to get core for " << model_name;
        return NULL;
    }

    // Versioned inference interface
    int infer(const char* model_name, const void* in, void* out, 
            uint32_t batch_size, uint64_t version) {
        auto it = _map.find(model_name);
        if (it == _map.end()) {
            LOG(WARNING) << "Cannot find engine in map, model name:"
                << model_name;
            return -1;
        }
        return it->second->infer(in, out, batch_size, version);
    }

    template<typename T>
    T* get_core(const char* model_name, uint64_t version) {
        auto it = _map.find(model_name);
        if (it == _map.end()) {
            LOG(WARNING) << "Cannot find engine in map, model name:"
                << model_name;
            return NULL;
        }
        return it->second->get_core<T>(version);
    }

    int query_version(const std::string& model, uint64_t& version) {
        auto it = _map.find(model);
        if (it == _map.end()) {
            LOG(WARNING) << "Cannot find engine in map, model name:" 
                << model;
            return -1;
        }
        auto infer_engine = it->second->default_engine();
        if (!infer_engine) {
            LOG(WARNING) << "Cannot get default engine for model:" 
                << model;
            return -1;
        }
        version = infer_engine->version();
        LOG(INFO) << "Succ get version: " << version << " for model: " 
            << model;
        return 0;
    }

private:
    boost::unordered_map<std::string, VersionedInferEngine*> _map;
};

} // predictor
} // paddle_serving
} // baidu

#endif // BAIDU_PADDLE_SERVING_PREDICTOR_INFER_H
