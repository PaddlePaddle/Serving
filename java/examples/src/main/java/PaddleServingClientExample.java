import io.paddle.serving.client.*;
import org.nd4j.linalg.api.ndarray.INDArray;
import org.nd4j.linalg.api.iter.NdIndexIterator;
import org.nd4j.linalg.factory.Nd4j;
import java.util.*;

public class PaddleServingClientExample {
    boolean fit_a_line() {
        float[] data = {0.0137f, -0.1136f, 0.2553f, -0.0692f,
            0.0582f, -0.0727f, -0.1583f, -0.0584f,
            0.6283f, 0.4919f, 0.1856f, 0.0795f, -0.0332f};
        INDArray npdata = Nd4j.createFromArray(data);
        HashMap<String, INDArray> feed_data
            = new HashMap<String, INDArray>() {{
                put("x", npdata);
            }};
        List<String> fetch = Arrays.asList("price");
        
        Client client = new Client();
        List<String> endpoints = Arrays.asList("localhost:9393");
        boolean succ = client.connect(endpoints);
        if (succ != true) {
            System.out.println("connect failed.");
            return false;
        }

        Map<String, INDArray> fetch_map = client.predict(feed_data, fetch);
        for (Map.Entry<String, INDArray> e : fetch_map.entrySet()) {
            System.out.println("Key = " + e.getKey() + ", Value = " + e.getValue());
        }
        return true;
    }

    boolean batch_predict() {
        float[] data = {0.0137f, -0.1136f, 0.2553f, -0.0692f,
            0.0582f, -0.0727f, -0.1583f, -0.0584f,
            0.6283f, 0.4919f, 0.1856f, 0.0795f, -0.0332f};
        INDArray npdata = Nd4j.createFromArray(data);
        HashMap<String, INDArray> feed_data
            = new HashMap<String, INDArray>() {{
                put("x", npdata);
            }};
        List<HashMap<String, INDArray>> feed_batch
            = new ArrayList<HashMap<String, INDArray>>() {{
                add(feed_data);
                add(feed_data);
            }};
        List<String> fetch = Arrays.asList("price");
        
        Client client = new Client();
        List<String> endpoints = Arrays.asList("localhost:9393");
        boolean succ = client.connect(endpoints);
        if (succ != true) {
            System.out.println("connect failed.");
            return false;
        }

        Map<String, INDArray> fetch_map = client.predict(feed_batch, fetch);
        for (Map.Entry<String, INDArray> e : fetch_map.entrySet()) {
            System.out.println("Key = " + e.getKey() + ", Value = " + e.getValue());
        }
        return true;
    }

    boolean asyn_predict() {
        float[] data = {0.0137f, -0.1136f, 0.2553f, -0.0692f,
            0.0582f, -0.0727f, -0.1583f, -0.0584f,
            0.6283f, 0.4919f, 0.1856f, 0.0795f, -0.0332f};
        INDArray npdata = Nd4j.createFromArray(data);
        HashMap<String, INDArray> feed_data
            = new HashMap<String, INDArray>() {{
                put("x", npdata);
            }};
        List<String> fetch = Arrays.asList("price");

        Client client = new Client();
        List<String> endpoints = Arrays.asList("localhost:9393");
        boolean succ = client.connect(endpoints);
        if (succ != true) {
            System.out.println("connect failed.");
            return false;
        }

        PredictFuture future = client.asyn_predict(feed_data, fetch);
        Map<String, INDArray> fetch_map = future.get();
        if (fetch_map == null) {
            System.out.println("Get future reslut failed");
            return false;
        }
        
        for (Map.Entry<String, INDArray> e : fetch_map.entrySet()) {
            System.out.println("Key = " + e.getKey() + ", Value = " + e.getValue());
        }
        return true;
    }

    boolean model_ensemble() {
        long[] data = {8, 233, 52, 601};
        INDArray npdata = Nd4j.createFromArray(data);
        HashMap<String, INDArray> feed_data
            = new HashMap<String, INDArray>() {{
                put("words", npdata);
            }};
        List<String> fetch = Arrays.asList("prediction");

        Client client = new Client();
        List<String> endpoints = Arrays.asList("localhost:9393");
        boolean succ = client.connect(endpoints);
        if (succ != true) {
            System.out.println("connect failed.");
            return false;
        }
        
        Map<String, HashMap<String, INDArray>> fetch_map
            = client.ensemble_predict(feed_data, fetch);
        for (Map.Entry<String, HashMap<String, INDArray>> entry : fetch_map.entrySet()) {
            System.out.println("Model = " + entry.getKey());
            HashMap<String, INDArray> tt = entry.getValue();
            for (Map.Entry<String, INDArray> e : tt.entrySet()) {
                System.out.println("Key = " + e.getKey() + ", Value = " + e.getValue());
            }
        }
        return true;
    }

    boolean bert() {
        float[] input_mask = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        long[] position_ids = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        long[] input_ids = {101, 6843, 3241, 749, 8024, 7662, 2533, 1391, 2533, 2523, 7676, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        long[] segment_ids = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        HashMap<String, INDArray> feed_data
            = new HashMap<String, INDArray>() {{
                put("input_mask", Nd4j.createFromArray(input_mask));
                put("position_ids", Nd4j.createFromArray(position_ids));
                put("input_ids", Nd4j.createFromArray(input_ids));
                put("segment_ids", Nd4j.createFromArray(segment_ids));
            }};
        List<String> fetch = Arrays.asList("pooled_output");

        Client client = new Client();
        List<String> endpoints = Arrays.asList("localhost:9393");
        boolean succ = client.connect(endpoints);
        if (succ != true) {
            System.out.println("connect failed.");
            return false;
        }
        
        Map<String, HashMap<String, INDArray>> fetch_map
            = client.ensemble_predict(feed_data, fetch);
        for (Map.Entry<String, HashMap<String, INDArray>> entry : fetch_map.entrySet()) {
            System.out.println("Model = " + entry.getKey());
            HashMap<String, INDArray> tt = entry.getValue();
            for (Map.Entry<String, INDArray> e : tt.entrySet()) {
                System.out.println("Key = " + e.getKey() + ", Value = " + e.getValue());
            }
        }
        return true;
    }

    public static void main( String[] args ) {
        // DL4J（Deep Learning for Java）Document:
        // https://www.bookstack.cn/read/deeplearning4j/bcb48e8eeb38b0c6.md
        PaddleServingClientExample e = new PaddleServingClientExample();
        boolean succ = false;
        
        for (String arg : args) {
            System.out.format("[Example] %s\n", arg);
            if ("fit_a_line".equals(arg)) {
                succ = e.fit_a_line();
            } else if ("bert".equals(arg)) {
                succ = e.bert();
            } else if ("model_ensemble".equals(arg)) {
                succ = e.model_ensemble();
            } else if ("asyn_predict".equals(arg)) {
                succ = e.asyn_predict();
            } else if ("batch_predict".equals(arg)) {
                succ = e.batch_predict();
            } else {
                System.out.format("%s not match: java -cp <jar> PaddleServingClientExample <exp>.\n", arg);
            }
        }

        if (succ == true) {
            System.out.println("[Example] succ.");
        } else {
            System.out.println("[Example] fail.");
        }
    }
}
