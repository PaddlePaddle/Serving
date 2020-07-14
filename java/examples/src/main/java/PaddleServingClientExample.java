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
        if (fetch_map == null) {
            return false;
        }

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
        if (fetch_map == null) {
            return false;
        }

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
        if (fetch_map == null) {
            return false;
        }

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
        
        Map<String, INDArray> fetch_map = client.predict(feed_data, fetch);
        if (fetch_map == null) {
            return false;
        }

        for (Map.Entry<String, INDArray> e : fetch_map.entrySet()) {
            System.out.println("Key = " + e.getKey() + ", Value = " + e.getValue());
        }
        return true;
    }

    boolean cube_local() {
        long[] embedding_14 = {250644};
        long[] embedding_2 = {890346};
        long[] embedding_10 = {3939};
        long[] embedding_17 = {421122};
        long[] embedding_23 = {664215};
        long[] embedding_6 = {704846};
        float[] dense_input = {0.0f, 0.006633499170812604f, 0.03f, 0.0f,
            0.145078125f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        long[] embedding_24 = {269955};
        long[] embedding_12 = {295309};
        long[] embedding_7 = {437731};
        long[] embedding_3 = {990128};
        long[] embedding_1 = {7753};
        long[] embedding_4 = {286835};
        long[] embedding_8 = {27346};
        long[] embedding_9 = {636474};
        long[] embedding_18 = {880474};
        long[] embedding_16 = {681378};
        long[] embedding_22 = {410878};
        long[] embedding_13 = {255651};
        long[] embedding_5 = {25207};
        long[] embedding_11 = {10891};
        long[] embedding_20 = {238459};
        long[] embedding_21 = {26235};
        long[] embedding_15 = {691460};
        long[] embedding_25 = {544187};
        long[] embedding_19 = {537425};
        long[] embedding_0 = {737395};

        HashMap<String, INDArray> feed_data
            = new HashMap<String, INDArray>() {{
                put("embedding_14.tmp_0", Nd4j.createFromArray(embedding_14));
                put("embedding_2.tmp_0", Nd4j.createFromArray(embedding_2));
                put("embedding_10.tmp_0", Nd4j.createFromArray(embedding_10));
                put("embedding_17.tmp_0", Nd4j.createFromArray(embedding_17));
                put("embedding_23.tmp_0", Nd4j.createFromArray(embedding_23));
                put("embedding_6.tmp_0", Nd4j.createFromArray(embedding_6));
                put("dense_input", Nd4j.createFromArray(dense_input));
                put("embedding_24.tmp_0", Nd4j.createFromArray(embedding_24));
                put("embedding_12.tmp_0", Nd4j.createFromArray(embedding_12));
                put("embedding_7.tmp_0", Nd4j.createFromArray(embedding_7));
                put("embedding_3.tmp_0", Nd4j.createFromArray(embedding_3));
                put("embedding_1.tmp_0", Nd4j.createFromArray(embedding_1));
                put("embedding_4.tmp_0", Nd4j.createFromArray(embedding_4));
                put("embedding_8.tmp_0", Nd4j.createFromArray(embedding_8));
                put("embedding_9.tmp_0", Nd4j.createFromArray(embedding_9));
                put("embedding_18.tmp_0", Nd4j.createFromArray(embedding_18));
                put("embedding_16.tmp_0", Nd4j.createFromArray(embedding_16));
                put("embedding_22.tmp_0", Nd4j.createFromArray(embedding_22));
                put("embedding_13.tmp_0", Nd4j.createFromArray(embedding_13));
                put("embedding_5.tmp_0", Nd4j.createFromArray(embedding_5));
                put("embedding_11.tmp_0", Nd4j.createFromArray(embedding_11));
                put("embedding_20.tmp_0", Nd4j.createFromArray(embedding_20));
                put("embedding_21.tmp_0", Nd4j.createFromArray(embedding_21));
                put("embedding_15.tmp_0", Nd4j.createFromArray(embedding_15));
                put("embedding_25.tmp_0", Nd4j.createFromArray(embedding_25));
                put("embedding_19.tmp_0", Nd4j.createFromArray(embedding_19));
                put("embedding_0.tmp_0", Nd4j.createFromArray(embedding_0));
            }};
        List<String> fetch = Arrays.asList("prob");

        Client client = new Client();
        List<String> endpoints = Arrays.asList("localhost:9292");
        boolean succ = client.connect(endpoints);
        if (succ != true) {
            System.out.println("connect failed.");
            return false;
        }
        
        Map<String, INDArray> fetch_map = client.predict(feed_data, fetch);
        if (fetch_map == null) {
            return false;
        }

        for (Map.Entry<String, INDArray> e : fetch_map.entrySet()) {
            System.out.println("Key = " + e.getKey() + ", Value = " + e.getValue());
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
            } else if ("cube_local".equals(arg)) {
                succ = e.cube_local();
            } else {
                System.out.format("%s not match: java -cp <jar> PaddleServingClientExample <exp>.\n", arg);
                System.out.println("<exp>: fit_a_line bert model_ensemble asyn_predict batch_predict cube_local.");
            }
        }

        if (succ == true) {
            System.out.println("[Example] succ.");
        } else {
            System.out.println("[Example] fail.");
        }
    }
}
