package io.paddle.serving.client;

import java.util.*;
import java.util.function.Function;
import java.lang.management.ManagementFactory;
import java.lang.management.RuntimeMXBean;

import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import io.grpc.StatusRuntimeException;
import com.google.protobuf.ByteString;

import com.google.common.util.concurrent.ListenableFuture;

import org.nd4j.linalg.api.ndarray.INDArray;
import org.nd4j.linalg.api.iter.NdIndexIterator;
import org.nd4j.linalg.factory.Nd4j;

import io.paddle.serving.grpc.*;
import io.paddle.serving.configure.*;
import io.paddle.serving.client.PredictFuture;

class Profiler {
    int pid_;
    String print_head_ = null;
    List<String> time_record_ = null;
    boolean enable_ = false;

    Profiler() {
        RuntimeMXBean runtimeMXBean = ManagementFactory.getRuntimeMXBean();
        pid_ = Integer.valueOf(runtimeMXBean.getName().split("@")[0]).intValue();
        print_head_ = "\nPROFILE\tpid:" + pid_ + "\t";
        time_record_ = new ArrayList<String>();
        time_record_.add(print_head_);
    }

    void record(String name) {
        if (enable_) {
            long ctime = System.currentTimeMillis() * 1000;
            time_record_.add(name + ":" + String.valueOf(ctime) + " ");
        }
    }

    void printProfile() {
        if (enable_) {
            String profile_str = String.join("", time_record_);
            time_record_ = new ArrayList<String>();
            time_record_.add(print_head_);
        }
    }

    void enable(boolean flag) {
        enable_ = flag;
    }
}

public class Client {
    private ManagedChannel channel_;
    private MultiLangGeneralModelServiceGrpc.MultiLangGeneralModelServiceBlockingStub blockingStub_;
    private MultiLangGeneralModelServiceGrpc.MultiLangGeneralModelServiceFutureStub futureStub_;
    private double rpcTimeoutS_;
    private List<String> feedNames_;
    private Map<String, Integer> feedTypes_;
    private Map<String, List<Integer>> feedShapes_;
    private List<String> fetchNames_;
    private Map<String, Integer> fetchTypes_;
    private Set<String> lodTensorSet_;
    private Map<String, Integer> feedTensorLen_;
    private Profiler profiler_;

    public Client() {
        channel_ = null;
        blockingStub_ = null;
        futureStub_ = null;
        rpcTimeoutS_ = 2;

        feedNames_ = null;
        feedTypes_ = null;
        feedShapes_ = null;
        fetchNames_ = null;
        fetchTypes_ = null;
        lodTensorSet_ = null;
        feedTensorLen_ = null;
        
        profiler_ = new Profiler();
        boolean is_profile = false;
        String FLAGS_profile_client = System.getenv("FLAGS_profile_client");
        if (FLAGS_profile_client != null && FLAGS_profile_client.equals("1")) {
            is_profile = true;
        }
        profiler_.enable(is_profile);
    }
    
    public boolean setRpcTimeoutMs(int rpc_timeout) {
        if (futureStub_ == null || blockingStub_ == null) {
            System.out.println("set timeout must be set after connect.");
            return false;
        }
        rpcTimeoutS_ = rpc_timeout / 1000.0;
        SetTimeoutRequest timeout_req = SetTimeoutRequest.newBuilder()
            .setTimeoutMs(rpc_timeout)
            .build();
        SimpleResponse resp;
        try {
            resp = blockingStub_.setTimeout(timeout_req);
        } catch (StatusRuntimeException e) {
            System.out.format("Set RPC timeout failed: %s\n", e.toString());
            return false;
        }
        return resp.getErrCode() == 0;
    }

    public boolean connect(String target) {
        // TODO: target must be NameResolver-compliant URI
        // https://grpc.github.io/grpc-java/javadoc/io/grpc/ManagedChannelBuilder.html
        try {
            channel_ = ManagedChannelBuilder.forTarget(target)
                .defaultLoadBalancingPolicy("round_robin")
                .maxInboundMessageSize(Integer.MAX_VALUE)
                .usePlaintext()
                .build();
            blockingStub_ = MultiLangGeneralModelServiceGrpc.newBlockingStub(channel_);
            futureStub_ = MultiLangGeneralModelServiceGrpc.newFutureStub(channel_);
        } catch (Exception e) {
            System.out.format("Connect failed: %s\n", e.toString());
            return false;
        }
        GetClientConfigRequest get_client_config_req = GetClientConfigRequest.newBuilder().build();
        GetClientConfigResponse resp;
        try {
            resp = blockingStub_.getClientConfig(get_client_config_req);
        } catch (Exception e) {
            System.out.format("Get Client config failed: %s\n", e.toString());
            return false;
        }
        String model_config_str = resp.getClientConfigStr();
        _parseModelConfig(model_config_str);
        return true;
    }

    private void _parseModelConfig(String model_config_str) {
        GeneralModelConfig.Builder model_conf_builder = GeneralModelConfig.newBuilder();
        try {
            com.google.protobuf.TextFormat.getParser().merge(model_config_str, model_conf_builder);
        } catch (com.google.protobuf.TextFormat.ParseException e) {
            System.out.format("Parse client config failed: %s\n", e.toString());
        }
        GeneralModelConfig model_conf = model_conf_builder.build();

        feedNames_ = new ArrayList<String>();
        fetchNames_ = new ArrayList<String>();
        feedTypes_ = new HashMap<String, Integer>();
        feedShapes_ = new HashMap<String, List<Integer>>();
        fetchTypes_ = new HashMap<String, Integer>();
        lodTensorSet_ = new HashSet<String>();
        feedTensorLen_ = new HashMap<String, Integer>();

        List<FeedVar> feed_var_list = model_conf.getFeedVarList();
        for (FeedVar feed_var : feed_var_list) {
            feedNames_.add(feed_var.getAliasName());
        }
        List<FetchVar> fetch_var_list = model_conf.getFetchVarList();
        for (FetchVar fetch_var : fetch_var_list) {
            fetchNames_.add(fetch_var.getAliasName());
        }

        for (int i = 0; i < feed_var_list.size(); ++i) {
            FeedVar feed_var = feed_var_list.get(i);
            String var_name = feed_var.getAliasName();
            feedTypes_.put(var_name, feed_var.getFeedType());
            feedShapes_.put(var_name, feed_var.getShapeList());
            if (feed_var.getIsLodTensor()) {
                lodTensorSet_.add(var_name);
            } else {
                int counter = 1;
                for (int dim : feedShapes_.get(var_name)) {
                    counter *= dim;
                }
                feedTensorLen_.put(var_name, counter);
            }
        }

        for (int i = 0; i < fetch_var_list.size(); i++) {
            FetchVar fetch_var = fetch_var_list.get(i);
            String var_name = fetch_var.getAliasName();
            fetchTypes_.put(var_name, fetch_var.getFetchType());
            if (fetch_var.getIsLodTensor()) {
                lodTensorSet_.add(var_name);
            }
        }
    }

    private InferenceRequest _packInferenceRequest(
            List<HashMap<String, INDArray>> feed_batch,
            Iterable<String> fetch) throws IllegalArgumentException {
        List<String> feed_var_names = new ArrayList<String>();
        feed_var_names.addAll(feed_batch.get(0).keySet());

        InferenceRequest.Builder req_builder = InferenceRequest.newBuilder()
            .addAllFeedVarNames(feed_var_names)
            .addAllFetchVarNames(fetch)
            .setIsPython(false);
        for (HashMap<String, INDArray> feed_data: feed_batch) {
            FeedInst.Builder inst_builder = FeedInst.newBuilder();
            for (String name: feed_var_names) {
                Tensor.Builder tensor_builder = Tensor.newBuilder();
                INDArray variable = feed_data.get(name);
                long[] flattened_shape = {-1};
                INDArray flattened_list = variable.reshape(flattened_shape);
                int v_type = feedTypes_.get(name);
                NdIndexIterator iter = new NdIndexIterator(flattened_list.shape());
                if (v_type == 0) { // int64
                    while (iter.hasNext()) {
                        long[] next_index = iter.next();
                        long x = flattened_list.getLong(next_index);
                        tensor_builder.addInt64Data(x);
                    }
                } else if (v_type == 1) { // float32
                    while (iter.hasNext()) {
                        long[] next_index = iter.next();
                        float x = flattened_list.getFloat(next_index);
                        tensor_builder.addFloatData(x);
                    }
                } else if (v_type == 2) { // int32
                    while (iter.hasNext()) {
                        long[] next_index = iter.next();
                        // the interface of INDArray is strange:
                        // https://deeplearning4j.org/api/latest/org/nd4j/linalg/api/ndarray/INDArray.html
                        int[] int_next_index = new int[next_index.length];
                        for(int i = 0; i < next_index.length; i++) {
                            int_next_index[i] = (int)next_index[i];
                        }
                        int x = flattened_list.getInt(int_next_index);
                        tensor_builder.addIntData(x);
                    }
                } else {
                    throw new IllegalArgumentException("error tensor value type.");
                }
                tensor_builder.addAllShape(feedShapes_.get(name));
                inst_builder.addTensorArray(tensor_builder.build());
            }
            req_builder.addInsts(inst_builder.build());
        }
        return req_builder.build();
    }

    private Map<String, HashMap<String, INDArray>>
        _unpackInferenceResponse(
            InferenceResponse resp,
            Iterable<String> fetch,
            Boolean need_variant_tag) throws IllegalArgumentException {
        return Client._staticUnpackInferenceResponse(
                resp, fetch, fetchTypes_, lodTensorSet_, need_variant_tag);
    }

    private static Map<String, HashMap<String, INDArray>>
        _staticUnpackInferenceResponse(
            InferenceResponse resp,
            Iterable<String> fetch,
            Map<String, Integer> fetchTypes,
            Set<String> lodTensorSet,
            Boolean need_variant_tag) throws IllegalArgumentException {
        if (resp.getErrCode() != 0) {
            return null;
        }
        String tag = resp.getTag();
        HashMap<String, HashMap<String, INDArray>> multi_result_map
            = new HashMap<String, HashMap<String, INDArray>>();
        for (ModelOutput model_result: resp.getOutputsList()) {
            String engine_name = model_result.getEngineName();
            FetchInst inst = model_result.getInsts(0);
            HashMap<String, INDArray> result_map
                = new HashMap<String, INDArray>();
            int index = 0;
            for (String name: fetch) {
                Tensor variable = inst.getTensorArray(index);
                int v_type = fetchTypes.get(name);
                INDArray data = null; 
                if (v_type == 0) { // int64
                    List<Long> list = variable.getInt64DataList();
                    long[] array = new long[list.size()];
                    for (int i = 0; i < list.size(); i++) {
                        array[i] = list.get(i);
                    }
                    data = Nd4j.createFromArray(array);
                } else if (v_type == 1) { // float32
                    List<Float> list = variable.getFloatDataList();
                    float[] array = new float[list.size()];
                    for (int i = 0; i < list.size(); i++) {
                        array[i] = list.get(i);
                    }
                    data = Nd4j.createFromArray(array);
                } else if (v_type == 2) { // int32
                    List<Integer> list = variable.getIntDataList();
                    int[] array = new int[list.size()];
                    for (int i = 0; i < list.size(); i++) {
                        array[i] = list.get(i);
                    }
                    data = Nd4j.createFromArray(array);
                } else {
                    throw new IllegalArgumentException("error tensor value type.");
                }
                // shape
                List<Integer> shape_lsit = variable.getShapeList();
                int[] shape_array = new int[shape_lsit.size()];
                for (int i = 0; i < shape_lsit.size(); ++i) {
                    shape_array[i] = shape_lsit.get(i);
                }
                data = data.reshape(shape_array);
                
                // put data to result_map
                result_map.put(name, data);

                // lod
                if (lodTensorSet.contains(name)) {
                    List<Integer> list = variable.getLodList();
                    int[] array = new int[list.size()];
                    for (int i = 0; i < list.size(); i++) {
                        array[i] = list.get(i);
                    }
                    result_map.put(name + ".lod", Nd4j.createFromArray(array));
                }
                index += 1;
            }
            multi_result_map.put(engine_name, result_map);
        }

        // TODO: tag(ABtest not support now)
        return multi_result_map;
    }

    public Map<String, INDArray> predict(
            HashMap<String, INDArray> feed,
            Iterable<String> fetch) {
        return predict(feed, fetch, false);
    }

    public Map<String, HashMap<String, INDArray>> ensemble_predict(
            HashMap<String, INDArray> feed,
            Iterable<String> fetch) {
        return ensemble_predict(feed, fetch, false);
    }

    public PredictFuture asyn_predict(
            HashMap<String, INDArray> feed,
            Iterable<String> fetch) {
        return asyn_predict(feed, fetch, false);
    }

    public Map<String, INDArray> predict(
            HashMap<String, INDArray> feed,
            Iterable<String> fetch,
            Boolean need_variant_tag) {
        List<HashMap<String, INDArray>> feed_batch
            = new ArrayList<HashMap<String, INDArray>>();
        feed_batch.add(feed);
        return predict(feed_batch, fetch, need_variant_tag);
    }
    
    public Map<String, HashMap<String, INDArray>> ensemble_predict(
            HashMap<String, INDArray> feed,
            Iterable<String> fetch,
            Boolean need_variant_tag) {
        List<HashMap<String, INDArray>> feed_batch
            = new ArrayList<HashMap<String, INDArray>>();
        feed_batch.add(feed);
        return ensemble_predict(feed_batch, fetch, need_variant_tag);
    }

    public PredictFuture asyn_predict(
            HashMap<String, INDArray> feed,
            Iterable<String> fetch,
            Boolean need_variant_tag) {
        List<HashMap<String, INDArray>> feed_batch
            = new ArrayList<HashMap<String, INDArray>>();
        feed_batch.add(feed);
        return asyn_predict(feed_batch, fetch, need_variant_tag);
    }

    public Map<String, INDArray> predict(
            List<HashMap<String, INDArray>> feed_batch,
            Iterable<String> fetch) {
        return predict(feed_batch, fetch, false);
    }
    
    public Map<String, HashMap<String, INDArray>> ensemble_predict(
            List<HashMap<String, INDArray>> feed_batch,
            Iterable<String> fetch) {
        return ensemble_predict(feed_batch, fetch, false);
    }

    public PredictFuture asyn_predict(
            List<HashMap<String, INDArray>> feed_batch,
            Iterable<String> fetch) {
        return asyn_predict(feed_batch, fetch, false);
    }

    public Map<String, INDArray> predict(
            List<HashMap<String, INDArray>> feed_batch,
            Iterable<String> fetch,
            Boolean need_variant_tag) {
        try {
            profiler_.record("java_prepro_0");
            InferenceRequest req = _packInferenceRequest(feed_batch, fetch);
            profiler_.record("java_prepro_1");
            
            profiler_.record("java_client_infer_0");
            InferenceResponse resp = blockingStub_.inference(req);
            profiler_.record("java_client_infer_1");

            profiler_.record("java_postpro_0");
            Map<String, HashMap<String, INDArray>> ensemble_result
                = _unpackInferenceResponse(resp, fetch, need_variant_tag);
            List<Map.Entry<String, HashMap<String, INDArray>>> list
                = new ArrayList<Map.Entry<String, HashMap<String, INDArray>>>(
                    ensemble_result.entrySet());
            if (list.size() != 1) {
                System.out.format("predict failed: please use ensemble_predict impl.\n");
                return null;
            }
            profiler_.record("java_postpro_1");
            profiler_.printProfile();

            return list.get(0).getValue();
        } catch (StatusRuntimeException e) {
            System.out.format("predict failed: %s\n", e.toString());
            return null;
        }
    }

    public Map<String, HashMap<String, INDArray>> ensemble_predict(
            List<HashMap<String, INDArray>> feed_batch,
            Iterable<String> fetch,
            Boolean need_variant_tag) {
        try {
            profiler_.record("java_prepro_0");
            InferenceRequest req = _packInferenceRequest(feed_batch, fetch);
            profiler_.record("java_prepro_1");
            
            profiler_.record("java_client_infer_0");
            InferenceResponse resp = blockingStub_.inference(req);
            profiler_.record("java_client_infer_1");
            
            profiler_.record("java_postpro_0");
            Map<String, HashMap<String, INDArray>> ensemble_result 
               = _unpackInferenceResponse(resp, fetch, need_variant_tag);
            profiler_.record("java_postpro_1");
            profiler_.printProfile();

            return ensemble_result;
        } catch (StatusRuntimeException e) {
            System.out.format("predict failed: %s\n", e.toString());
            return null;
        }
    }

    public PredictFuture asyn_predict(
            List<HashMap<String, INDArray>> feed_batch,
            Iterable<String> fetch,
            Boolean need_variant_tag) {
        InferenceRequest req = _packInferenceRequest(feed_batch, fetch);
        ListenableFuture<InferenceResponse> future = futureStub_.inference(req);
        PredictFuture predict_future = new PredictFuture(future, 
            (InferenceResponse resp) -> {
                return Client._staticUnpackInferenceResponse(
                resp, fetch, fetchTypes_, lodTensorSet_, need_variant_tag);
            }
        );
        return predict_future;
    }
}
