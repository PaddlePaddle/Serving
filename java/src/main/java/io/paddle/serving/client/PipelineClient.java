package io.paddle.serving.pipelineclient;

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

import io.paddle.serving.pipelineproto.*;
import io.paddle.serving.pipelineclient.PipelineFuture;


/**
* PipelineClient class defination
* @author HexToString
*/
public class PipelineClient {
    private ManagedChannel channel_;
    private PipelineServiceGrpc.PipelineServiceBlockingStub blockingStub_;
    private PipelineServiceGrpc.PipelineServiceFutureStub futureStub_;
    private String clientip;

    private String _profile_key;
    private String _profile_value;
    
    public PipelineClient() {
        channel_ = null;
        blockingStub_ = null;
        futureStub_ = null;
        boolean is_profile = false;
        clientip = null;
        _profile_value = "1";
        _profile_key = "pipeline.profile";
    }
    
    /**
   * This method returns the sign of connect status.
   * @param target String type(The server ipv4 and port) such as "192.168.10.10:8891".
   * @return boolean (the sign of connect status).
   */
    public boolean connect(String target) {
        try {
            String[] temp = target.split(":");
            this.clientip = temp[0] == "localhost"?"127.0.0.1":temp[0];
            channel_ = ManagedChannelBuilder.forTarget(target)
                .defaultLoadBalancingPolicy("round_robin")
                .maxInboundMessageSize(Integer.MAX_VALUE)
                .usePlaintext()
                .build();
            blockingStub_ = PipelineServiceGrpc.newBlockingStub(channel_);
            futureStub_ = PipelineServiceGrpc.newFutureStub(channel_);
        } catch (Exception e) {
            System.out.format("Connect failed: %s\n", e.toString());
            return false;
        }
        return true;
    }

    /**
   * This method returns the Packaged Request.
   * @param feed_dict HashMap<String, String>(input data).
   * @param profile boolean(profile sign).
   * @param logid int
   * @return Request (the grpc protobuf Request).
   */
    private Request _packInferenceRequest(
            HashMap<String, String> feed_dict,
            boolean profile,
            int logid) throws IllegalArgumentException {
        List<String> keys = new ArrayList<String>();
        List<String> values = new ArrayList<String>();
        long[] flattened_shape = {-1};
        
        Request.Builder req_builder = Request.newBuilder()
            .setClientip(this.clientip)
            .setLogid(logid);
        for (Map.Entry<String, String> entry : feed_dict.entrySet()) {
            keys.add(entry.getKey());
            values.add(entry.getValue());
        }
        if (profile) {
            keys.add(_profile_key);
            values.add(_profile_value);
        }
        req_builder.addAllKey(keys);
        req_builder.addAllValue(values);
        return req_builder.build();
    }

    /**
   * This method returns the HashMap which is unpackaged from Response.
   * @param resp Response(the grpc protobuf Response).
   * @return HashMap<String,String> (the output).
   */
    private HashMap<String,String> _unpackResponse(Response resp) throws IllegalArgumentException{
        return PipelineClient._staitcUnpackResponse(resp);
    }

    /**
   * This static method returns the HashMap which is unpackaged from Response.
   * @param resp Response(the grpc protobuf Response).
   * @return HashMap<String,String> (the output).
   */
    private static HashMap<String,String> _staitcUnpackResponse(Response resp) {
        HashMap<String,String> ret_Map = new HashMap<String,String>();
        int err_no  = resp.getErrNo();
        if ( err_no!= 0) {
            return null;
        }
        List<String> keys = resp.getKeyList();
        List<String> values= resp.getValueList();
        for (int i = 0;i<keys.size();i++) {
            ret_Map.put(keys.get(i),values.get(i));
        }
        return ret_Map;
    }

    /**
   * The synchronous prediction method.
   * @param feed_batch HashMap<String, String>(input data).
   * @param fetch Iterable<String>(the output key list).
   * @param profile boolean(profile sign).
   * @param logid int
   * @return HashMap<String,String> (the output).
   */
    public HashMap<String,String> predict(
            HashMap<String, String> feed_batch,
            Iterable<String> fetch,
            boolean profile,
            int logid) {
        try {
            Request req = _packInferenceRequest(
                    feed_batch, profile,logid);
            Response resp = blockingStub_.inference(req);
            return _unpackResponse(resp);
        } catch (StatusRuntimeException e) {
            System.out.format("Failed to predict: %s\n", e.toString());
            return null;
        }
    }

    /**
   * The synchronous prediction overload function.
   */
    public HashMap<String,String> predict(
            HashMap<String, String> feed_batch,
            Iterable<String> fetch) {
                return predict(feed_batch,fetch,false,0);
    }

    /**
   * The synchronous prediction overload function.
   */
    public HashMap<String,String> predict(
            HashMap<String, String> feed_batch,
            Iterable<String> fetch,
            boolean profile) {
                return predict(feed_batch,fetch,profile,0);
    }

    /**
   * The synchronous prediction overload function.
   */
    public HashMap<String,String> predict(
            HashMap<String, String> feed_batch,
            Iterable<String> fetch,
            int logid) {
                return predict(feed_batch,fetch,false,logid);
    }

    /**
   * The asynchronous prediction method.use future.get() to get the result.
   * @param feed_batch HashMap<String, String>(input data).
   * @param fetch Iterable<String>(the output key list).
   * @param profile boolean(profile sign).
   * @param logid int
   * @return PipelineFuture(the output future). 
   */
    public PipelineFuture asyn_predict(
            HashMap<String, String> feed_batch,
            Iterable<String> fetch,
            boolean profile,
            int logid) {
        Request req = _packInferenceRequest(
                feed_batch, profile, logid);
        ListenableFuture<Response> future = futureStub_.inference(req);
        PipelineFuture predict_future = new PipelineFuture(future, 
            (Response resp) -> {
                return PipelineClient._staitcUnpackResponse(resp);
            }
        );
        return predict_future;
    }

    /**
   * The asynchronous prediction overload function.
   */
    public PipelineFuture asyn_predict(
            HashMap<String, String> feed_batch,
            Iterable<String> fetch) {
                return asyn_predict(feed_batch,fetch,false,0);
    }

    /**
   * The asynchronous prediction overload function.
   */
    public PipelineFuture asyn_predict(
            HashMap<String, String> feed_batch,
            Iterable<String> fetch,
            boolean profile) {
                return asyn_predict(feed_batch,fetch,profile,0);
    }

    /**
   * The asynchronous prediction overload function.
   */
    public PipelineFuture asyn_predict(
            HashMap<String, String> feed_batch,
            Iterable<String> fetch,
            int logid) {
                return asyn_predict(feed_batch,fetch,false,logid);
    }


}
