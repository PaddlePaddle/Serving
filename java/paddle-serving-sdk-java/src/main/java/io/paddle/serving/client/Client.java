package io.paddle.serving.client;

import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import io.grpc.StatusRuntimeException;
import io.paddle.serving.grpc.*;


public class Client {
    private ManagedChannel channel_;
    private MultiLangGeneralModelServiceGrpc.MultiLangGeneralModelServiceBlockingStub blockingStub_;
    private MultiLangGeneralModelServiceGrpc.MultiLangGeneralModelServiceStub asyncStub_;
    private double rpcTimeoutS_;

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

    }

    public static void main( String[] args ) {
        System.out.println( "Hello World!" );
    }
}
