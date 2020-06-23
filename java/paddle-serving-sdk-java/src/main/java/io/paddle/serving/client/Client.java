package io.paddle.serving.client;

import java.util.*;
import java.util.function.Function;

import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import io.grpc.StatusRuntimeException;

import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;

import io.paddle.serving.grpc.*;
import io.paddle.serving.configure.*;


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

    Client() {
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
    }

    public Boolean setRpcTimeoutMs(int rpc_timeout) throws NullPointerException {
        if (futureStub_ == null || blockingStub_ == null) {
            throw new NullPointerException("set timeout must be set after connect.");
        }
        rpcTimeoutS_ = rpc_timeout / 1000.0;
        SetTimeoutRequest timeout_req = SetTimeoutRequest.newBuilder()
            .setTimeoutMs(rpc_timeout)
            .build();
        SimpleResponse resp;
        try {
            resp = blockingStub_.setTimeout(timeout_req);
        } catch (StatusRuntimeException e) {
            System.out.format("Set RPC timeout failed: %s", e.toString());
            return false;
        }
        return resp.getErrCode() == 0;
    }

    public Boolean connect(List<String> endpoints) {
        String target = "ipv4:" + String.join(",", endpoints);
        // TODO: max_receive_message_length and max_send_message_length
        try {
            channel_ = ManagedChannelBuilder.forTarget(target)
                .defaultLoadBalancingPolicy("round_robin")
                .usePlaintext()
                .build();
            blockingStub_ = MultiLangGeneralModelServiceGrpc.newBlockingStub(channel_);
            futureStub_ = MultiLangGeneralModelServiceGrpc.newFutureStub(channel_);
        } catch (Exception e) {
            System.out.format("Connect failed: %s", e.toString());
            return false;
        }
        GetClientConfigRequest get_client_config_req = GetClientConfigRequest.newBuilder().build();
        GetClientConfigResponse resp;
        try {
            resp = blockingStub_.getClientConfig(get_client_config_req);
        } catch (StatusRuntimeException e) {
            System.out.format("Get Client config failed: %s", e.toString());
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
            System.out.format("Parse client config failed: %s", e.toString());
        }
        GeneralModelConfig model_conf = model_conf_builder.build();

        feedNames_ = new ArrayList<String>();
        fetchNames_ = new ArrayList<String>();
        feedTypes_ = new HashMap<String, Integer>();
        feedShapes_ = new HashMap<String, List<Integer>>();
        fetchTypes_ = new HashMap<String, Integer>();
        lodTensorSet_ = new HashSet<String>();

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
                // TODO: check shape
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

    private List<? extends Number> _flattenList(List<? extends Number> x) {
        // TODO
        return x;
    }

    private InferenceRequest _packInferenceRequest(
            List<Map<String, List<? extends Number>>> feed_batch,
            Iterable<String> fetch) throws IllegalArgumentException {
        List<String> feed_var_names = new ArrayList<String>();
        feed_var_names.addAll(feed_batch.get(0).keySet());

        InferenceRequest.Builder req_builder = InferenceRequest.newBuilder()
            .addAllFeedVarNames(feed_var_names)
            .addAllFetchVarNames(fetch)
            .setIsPython(false);
        for (Map<String, List<? extends Number>> feed_data: feed_batch) {
            FeedInst.Builder inst_builder = FeedInst.newBuilder();
            for (String name: feed_var_names) {
                Tensor.Builder tensor_builder = Tensor.newBuilder();
                List<? extends Number> variable = feed_data.get(name);
                List<? extends Number> flattened_list = _flattenList(variable);
                int v_type = feedTypes_.get(name);
                if (v_type == 0) { // int64
                    for (Number x: flattened_list) {
                        tensor_builder.addInt64Data((long)x);
                    }
                } else if (v_type == 1) { // float32
                    for (Number x: flattened_list) {
                        tensor_builder.addFloatData((float)x);
                    }
                } else if (v_type == 2) { // int32
                    for (Number x: flattened_list) {
                        tensor_builder.addIntData((int)x);
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

    private Map<String, ? extends Map<String, List<? extends Number>>>
        _unpackInferenceResponse(
            InferenceResponse resp,
            Iterable<String> fetch,
            Boolean need_variant_tag) throws IllegalArgumentException {
        return Client._staticUnpackInferenceResponse(
                resp, fetch, fetchTypes_, lodTensorSet_, need_variant_tag);
    }

    private static Map<String, ? extends Map<String, List<? extends Number>>>
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
        Map<String, ? extends Map<String, List<? extends Number>>> multi_result_map
            = new HashMap<String, HashMap<String, List<? extends Number>>>();
        for (ModelOutput model_result: resp.getOutputsList()) {
            FetchInst inst = model_result.getInsts(0);
            Map<String, List<? extends Number>> result_map
                = new HashMap<String, List<? extends Number>>();
            int index = 0;
            for (String name: fetch) {
                Tensor variable = inst.getTensorArray(index);
                int v_type = fetchTypes.get(name);
                if (v_type == 0) { // int64
                    result_map.put(name, variable.getInt64DataList());
                } else if (v_type == 1) { // float32
                    result_map.put(name, variable.getFloatDataList());
                } else if (v_type == 2) { // int32
                    result_map.put(name, variable.getIntDataList());
                } else {
                    throw new IllegalArgumentException("error tensor value type.");
                }
                // TODO: shape

                if (lodTensorSet.contains(name)) {
                    result_map.put(name + ".lod", variable.getLodList());
                }
                index += 1;
            }
        }

        // TODO: tag
        return multi_result_map;
    }

    /*
    public Map<String, ? extends Map<String, List<? extends Number>>> predict(
            Map<String, List<? extends Number>> feed,
            Iterable<String> fetch) {
        return predict(feed, fetch, false);
    }

    public PredictFuture async_predict(
            Map<String, List<? extends Number>> feed,
            Iterable<String> fetch) {
        return async_predict(feed, fetch, false);
    }

    public Map<String, ? extends Map<String, List<? extends Number>>> predict(
            Map<String, List<? extends Number>> feed,
            Iterable<String> fetch,
            Boolean need_variant_tag) {
        List<? extends Map<String, List<? extends Number>>> feed_batch
            = new ArrayList<? extends Map<String, List<? extends Number>>>();
        feed_batch.add(feed);
        return predict(feed_batch, fetch, need_variant_tag);
    }

    public PredictFuture async_predict(
            Map<String, List<? extends Number>> feed,
            Iterable<String> fetch,
            Boolean need_variant_tag) {
        List<? extends Map<String, List<? extends Number>>> feed_batch
            = new ArrayList<? extends Map<String, List<? extends Number>>>();
        feed_batch.add(feed);
        return async_predict(feed_batch, fetch, need_variant_tag);
    }
    */

    public Map<String, ? extends Map<String, List<? extends Number>>> predict(
            List<? extends Map<String, List<? extends Number>>> feed_batch,
            Iterable<String> fetch) {
        return predict(feed_batch, fetch, false);
    }

    public PredictFuture async_predict(
            List<? extends Map<String, List<? extends Number>>> feed_batch,
            Iterable<String> fetch) {
        return async_predict(feed_batch, fetch, false);
    }

    public Map<String, ? extends Map<String, List<? extends Number>>> predict(
            List<? extends Map<String, List<? extends Number>>> feed_batch,
            Iterable<String> fetch,
            Boolean need_variant_tag) {
        InferenceRequest req = _packInferenceRequest(feed_batch, fetch);
        try {
            InferenceResponse resp = blockingStub_.inference(req);
            return _unpackInferenceResponse(
                    resp, fetch, need_variant_tag);
        } catch (StatusRuntimeException e) {
            System.out.format("grpc failed: %s", e.toString());
            return Collections.emptyMap();
        }
    }

    public PredictFuture async_predict(
            List<Map<String, List<? extends Number>>> feed_batch,
            Iterable<String> fetch,
            Boolean need_variant_tag) {
        InferenceRequest req = _packInferenceRequest(feed_batch, fetch);
        ListenableFuture<InferenceResponse> future = futureStub_.inference(req);
        return new PredictFuture(
                future, 
                (InferenceResponse resp) -> {
                    return Client._staticUnpackInferenceResponse(
                    resp, fetch, fetchTypes_, lodTensorSet_, need_variant_tag);
                });
    }

    public static void main( String[] args ) {
        Client client = new Client();
        List<String> endpoints = new ArrayList<String>()
            .add("182.61.111.54:9393");
        Client.connect(endpoints);
        List<HashMap<String, List<Float>>> feed_batch
            = new ArrayList<HashMap<String, List<Float>>>()
            .add(new HashMap<String, ArrayList<Float>>()
                    .put("x", new ArrayList<Float>()
                        .add(0.0137f).add(-0.1136f).add(0.2553f)
                        .add(-0.0692f).add(0.0582f).add(-0.0727f)
                        .add(-0.1583f).add(-0.0584f).add(0.6283f)
                        .add(0.4919f).add(0.1856f).add(0.0795f)
                        .add(-0.0332f)));
        List<String> fetch = new ArrayList<String>()
            .add("price");
        Map<String, ? extends Map<String, List<?>>> fetch_map
            = client.predict(feed_batch, fetch);
        System.out.println( "Hello World!" );
    }
}

class PredictFuture {
    private ListenableFuture<InferenceResponse> callFuture_;
    private Function<InferenceResponse, 
                     Map<String, ? extends Map<String, List<? extends Number>>>> callBackFunc_;
    
    PredictFuture(ListenableFuture<InferenceResponse> call_future,
            Function<InferenceResponse, 
                    Map<String, ? extends Map<String, List<? extends Number>>>> call_back_func) {
        callFuture_ = call_future;
        callBackFunc_ = call_back_func;
    }

    public Map<String, ? extends Map<String, List<? extends Number>>> get() throws Exception {
        InferenceResponse resp = null;
        try {
            resp = callFuture_.get();
        } catch (Exception e) {
            System.out.format("grpc failed: %s", e.toString());
            return null;
        }
        return callBackFunc_.apply(resp);
    }
} 
