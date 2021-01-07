package io.paddle.serving.pipelineclient;

import java.util.*;
import java.util.function.Function;
import io.grpc.StatusRuntimeException;
import com.google.common.util.concurrent.ListenableFuture;
import org.nd4j.linalg.api.ndarray.INDArray;

import io.paddle.serving.pipelineclient.PipelineClient;
import io.paddle.serving.pipelineproto.*;

/**
* PipelineFuture class is for asynchronous prediction
* @author HexToString
*/
public class PipelineFuture {
    private ListenableFuture<Response> callFuture_;
    private Function<Response, 
        HashMap<String,String> > callBackFunc_;
    
        PipelineFuture(ListenableFuture<Response> call_future,
            Function<Response, 
            HashMap<String,String> > call_back_func) {
        callFuture_ = call_future;
        callBackFunc_ = call_back_func;
    }

    /**
    * use this method to get the result of asynchronous prediction.
    */
    public HashMap<String,String> get() {
        Response resp = null;
        try {
            resp = callFuture_.get();
        } catch (Exception e) {
            System.out.format("predict failed: %s\n", e.toString());
            return null;
        }
        HashMap<String,String> result
            = callBackFunc_.apply(resp);
        return result;
    }
}
