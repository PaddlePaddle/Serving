package io.paddle.serving.client;

import java.util.*;

import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import io.grpc.StatusRuntimeException;
//import com.google.protobuf;

import io.paddle.serving.grpc.*;
import io.paddle.serving.configure.*;


public class Client {
    private ManagedChannel channel_;
    private MultiLangGeneralModelServiceGrpc.MultiLangGeneralModelServiceBlockingStub blockingStub_;
    private MultiLangGeneralModelServiceGrpc.MultiLangGeneralModelServiceStub asyncStub_;
    private double rpcTimeoutS_;
    private List<String> feedNames_;
    private Map<String, Integer> feedTypes_;
    private Map<String, List<Integer>> feedShapes_;
    private List<String> fetchNames_;
    private Map<String, Integer> fetchTypes_;
    private Map<String, List<Integer>> fetchShapes_;
    private Set<String> lodTensorSet_;

    Client() {
        channel_ = null;
        blockingStub_ = null;
        asyncStub_ = null;
        rpcTimeoutS_ = 2;
    }

    Boolean setRpcTimeoutMs(int rpc_timeout) throws NullPointerException {
        if (asyncStub_ == null || blockingStub_ == null) {
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

    Boolean connect(String[] endpoints) {
        String target = "ipv4:" + String.join(",", endpoints);
        // TODO: max_receive_message_length and max_send_message_length
        try {
            channel_ = ManagedChannelBuilder.forTarget(target)
                .defaultLoadBalancingPolicy("round_robin")
                .usePlaintext()
                .build();
            blockingStub_ = MultiLangGeneralModelServiceGrpc.newBlockingStub(channel_);
            asyncStub_ = MultiLangGeneralModelServiceGrpc.newStub(channel_);
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

    void _parseModelConfig(String model_config_str) {
        GeneralModelConfig.Builder model_conf_builder = GeneralModelConfig.newBuilder();
        try {
            com.google.protobuf.TextFormat.getParser().merge(model_config_str, model_conf_builder);
        } catch (com.google.protobuf.TextFormat.ParseException e) {
            System.out.format("Parse client config failed: %s", e.toString());
        }
        GeneralModelConfig model_conf = model_conf_builder.build();

        List<String> feedNames_ ;
        List<FeedVar> feed_var_list = model_conf.getFeedVarList();
        for (FeedVar feed_var : feed_var_list) {
            feedNames_.add(feed_var.getAliasName());
        }
        List<String> fetchNames_ ;
        List<FetchVar> fetch_var_list = model_conf.getFetchVarList();
        for (FetchVar fetch_var : fetch_var_list) {
            fetchNames_.add(fetch_var.getAliasName());
        }
        feedTypes_ = new HashMap<String, Integer>();
        feedShapes_ = new HashMap<String, List<Integer>>();
        fetchTypes_ = new HashMap<String, Integer>();
        fetchShapes_ = new HashMap<String, List<Integer>>();
        lodTensorSet_ = new HashSet<String>();

        for (int i = 0; i < feed_var_list.size(); ++i) {
            FeedVar feed_var = feed_var_list[i];
            String var_name = feed_var.getAliasName();
            // feedTypes_[var_name] = feed_var.getFeedType();
        }
    }

    public static void main( String[] args ) {
        System.out.println( "Hello World!" );
    }
}
