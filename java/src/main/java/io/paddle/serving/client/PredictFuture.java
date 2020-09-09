package io.paddle.serving.client;

import java.util.*;
import java.util.function.Function;
import io.grpc.StatusRuntimeException;
import com.google.common.util.concurrent.ListenableFuture;
import org.nd4j.linalg.api.ndarray.INDArray;

import io.paddle.serving.client.Client;
import io.paddle.serving.grpc.*;

public class PredictFuture {
    private ListenableFuture<InferenceResponse> callFuture_;
    private Function<InferenceResponse, 
                     Map<String, HashMap<String, INDArray>>> callBackFunc_;
    
    PredictFuture(ListenableFuture<InferenceResponse> call_future,
            Function<InferenceResponse, 
                     Map<String, HashMap<String, INDArray>>> call_back_func) {
        callFuture_ = call_future;
        callBackFunc_ = call_back_func;
    }

    public Map<String, INDArray> get() {
        InferenceResponse resp = null;
        try {
            resp = callFuture_.get();
        } catch (Exception e) {
            System.out.format("predict failed: %s\n", e.toString());
            return null;
        }
        Map<String, HashMap<String, INDArray>> ensemble_result
            = callBackFunc_.apply(resp);
        List<Map.Entry<String, HashMap<String, INDArray>>> list
            = new ArrayList<Map.Entry<String, HashMap<String, INDArray>>>(
                    ensemble_result.entrySet());
        if (list.size() != 1) {
            System.out.format("predict failed: please use get_ensemble impl.\n");
            return null;
        }
        return list.get(0).getValue();
    }

    public Map<String, HashMap<String, INDArray>> ensemble_get() {
        InferenceResponse resp = null;
        try {
            resp = callFuture_.get();
        } catch (Exception e) {
            System.out.format("predict failed: %s\n", e.toString());
            return null;
        }
        return callBackFunc_.apply(resp);
    }
}
