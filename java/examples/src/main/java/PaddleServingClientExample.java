import io.paddle.serving.client.*;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import org.nd4j.linalg.api.iter.NdIndexIterator;
import org.nd4j.linalg.api.ndarray.INDArray;
import org.datavec.image.loader.NativeImageLoader;
import org.nd4j.linalg.api.ops.CustomOp;
import org.nd4j.linalg.api.ops.DynamicCustomOp;
import org.nd4j.linalg.factory.Nd4j;
import java.util.*;

public class PaddleServingClientExample {
    boolean http_proto(String model_config_path) {
        float[] data = {0.0137f, -0.1136f, 0.2553f, -0.0692f,
            0.0582f, -0.0727f, -0.1583f, -0.0584f,
            0.6283f, 0.4919f, 0.1856f, 0.0795f, -0.0332f};
        INDArray npdata = Nd4j.createFromArray(data);
        long[] batch_shape = {1,13};
        INDArray batch_npdata = npdata.reshape(batch_shape);
        HashMap<String, Object> feed_data
            = new HashMap<String, Object>() {{
                put("x", batch_npdata);
            }};
        List<String> fetch = Arrays.asList("price");
        
        Client client = new Client();
        client.setIP("127.0.0.1");
        client.setPort("9393");
        client.loadClientConfig(model_config_path);
        String result = client.predict(feed_data, fetch, true, 0);
        
        System.out.println(result);
        return true;
    }

    boolean http_json(String model_config_path) {
        float[] data = {0.0137f, -0.1136f, 0.2553f, -0.0692f,
            0.0582f, -0.0727f, -0.1583f, -0.0584f,
            0.6283f, 0.4919f, 0.1856f, 0.0795f, -0.0332f};
        INDArray npdata = Nd4j.createFromArray(data);
        long[] batch_shape = {1,13};
        INDArray batch_npdata = npdata.reshape(batch_shape);
        HashMap<String, Object> feed_data
            = new HashMap<String, Object>() {{
                put("x", batch_npdata);
            }};
        List<String> fetch = Arrays.asList("price");
        
        Client client = new Client();
        //注意：跨docker，需要设置--net-host或直接访问另一个docker的ip
        client.setIP("127.0.0.1");
        client.setPort("9393");
        client.set_http_proto(false);
        client.loadClientConfig(model_config_path);
        String result = client.predict(feed_data, fetch, true, 0);
        
        System.out.println(result);
        return true;
    }

    boolean grpc(String model_config_path) {
        float[] data = {0.0137f, -0.1136f, 0.2553f, -0.0692f,
            0.0582f, -0.0727f, -0.1583f, -0.0584f,
            0.6283f, 0.4919f, 0.1856f, 0.0795f, -0.0332f};
        INDArray npdata = Nd4j.createFromArray(data);
        long[] batch_shape = {1,13};
        INDArray batch_npdata = npdata.reshape(batch_shape);
        HashMap<String, Object> feed_data
            = new HashMap<String, Object>() {{
                put("x", batch_npdata);
            }};
        List<String> fetch = Arrays.asList("price");
        
        Client client = new Client();
        client.setIP("127.0.0.1");
        client.setPort("9393");
        client.loadClientConfig(model_config_path);
        client.set_use_grpc_client(true);
        String result = client.predict(feed_data, fetch, true, 0);
        
        System.out.println(result);
        return true;
    }

    boolean encrypt(String model_config_path,String keyFilePath) {
        float[] data = {0.0137f, -0.1136f, 0.2553f, -0.0692f,
            0.0582f, -0.0727f, -0.1583f, -0.0584f,
            0.6283f, 0.4919f, 0.1856f, 0.0795f, -0.0332f};
        INDArray npdata = Nd4j.createFromArray(data);
        long[] batch_shape = {1,13};
        INDArray batch_npdata = npdata.reshape(batch_shape);
        HashMap<String, Object> feed_data
            = new HashMap<String, Object>() {{
                put("x", batch_npdata);
            }};
        List<String> fetch = Arrays.asList("price");
        
        Client client = new Client();
        client.setIP("127.0.0.1");
        client.setPort("9393");
        client.loadClientConfig(model_config_path);
        client.use_key(keyFilePath);
        try {
            Thread.sleep(1000*3);   // 休眠3秒，等待Server启动
        } catch (Exception e) {
        }
        String result = client.predict(feed_data, fetch, true, 0);
        
        System.out.println(result);
        return true;
    }

    boolean compress(String model_config_path) {
        float[] data = {0.0137f, -0.1136f, 0.2553f, -0.0692f,
            0.0582f, -0.0727f, -0.1583f, -0.0584f,
            0.6283f, 0.4919f, 0.1856f, 0.0795f, -0.0332f};
        INDArray npdata = Nd4j.createFromArray(data);
        long[] batch_shape = {500,13};
        INDArray batch_npdata = npdata.broadcast(batch_shape);
        HashMap<String, Object> feed_data
            = new HashMap<String, Object>() {{
                put("x", batch_npdata);
            }};
        List<String> fetch = Arrays.asList("price");
        
        Client client = new Client();
        client.setIP("127.0.0.1");
        client.setPort("9393");
        client.loadClientConfig(model_config_path);
        client.set_request_compress(true);
        client.set_response_compress(true);
        String result = client.predict(feed_data, fetch, true, 0);
        System.out.println(result);
        return true;
    }

    boolean yolov4(String model_config_path,String filename) {
        // https://deeplearning4j.konduit.ai/
        int height = 608;
        int width = 608;
        int channels = 3;
        NativeImageLoader loader = new NativeImageLoader(height, width, channels);
        INDArray BGRimage = null;
        try {
            BGRimage = loader.asMatrix(new File(filename));
        } catch (java.io.IOException e) {
            System.out.println("load image fail.");
            return false;
        }   

        // shape: (channels, height, width)
        BGRimage = BGRimage.reshape(channels, height, width);
        INDArray RGBimage = Nd4j.create(BGRimage.shape());

        // BGR2RGB
        CustomOp op = DynamicCustomOp.builder("reverse")
            .addInputs(BGRimage)
            .addOutputs(RGBimage)
            .addIntegerArguments(0)
            .build();
        Nd4j.getExecutioner().exec(op);
        
        // Div(255.0)
        INDArray image = RGBimage.divi(255.0);
        long[] batch_shape = {1,image.shape()[0],image.shape()[1],image.shape()[2]};
        INDArray batch_image = image.reshape(batch_shape);        

        INDArray im_size = Nd4j.createFromArray(new int[]{height, width});
        long[] batch_size_shape = {1,2};
        INDArray batch_im_size = im_size.reshape(batch_size_shape);
        HashMap<String, Object> feed_data
            = new HashMap<String, Object>() {{
                put("image", batch_image);
                put("im_size", batch_im_size);
            }};
        List<String> fetch = Arrays.asList("save_infer_model/scale_0.tmp_0");
        Client client = new Client();
        client.setIP("127.0.0.1");
        client.setPort("9393");
        client.loadClientConfig(model_config_path);
        String result = client.predict(feed_data, fetch, true, 0);
        System.out.println(result);
        return true;
    }

    boolean bert(String model_config_path) {
        float[] input_mask = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        long[] position_ids = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        long[] input_ids = {101, 6843, 3241, 749, 8024, 7662, 2533, 1391, 2533, 2523, 7676, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        long[] segment_ids = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        HashMap<String, Object> feed_data
            = new HashMap<String, Object>() {{
                put("input_mask", Nd4j.createFromArray(input_mask));
                put("position_ids", Nd4j.createFromArray(position_ids));
                put("input_ids", Nd4j.createFromArray(input_ids));
                put("segment_ids", Nd4j.createFromArray(segment_ids));
            }};
        List<String> fetch = Arrays.asList("pooled_output");
        Client client = new Client();
        client.setIP("127.0.0.1");
        client.setPort("9393");
        client.loadClientConfig(model_config_path);
        String result = client.predict(feed_data, fetch, true, 0);
        System.out.println(result);
        return true;
    }

    boolean cube_local(String model_config_path) {
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

        HashMap<String, Object> feed_data
            = new HashMap<String, Object>() {{
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
        client.setIP("127.0.0.1");
        client.setPort("9393");
        client.loadClientConfig(model_config_path);
        String result = client.predict(feed_data, fetch, true, 0);
        System.out.println(result);
        return true;
    }

    public static void main( String[] args ) {
        // DL4J（Deep Learning for Java）Document:
        // https://www.bookstack.cn/read/deeplearning4j/bcb48e8eeb38b0c6.md
        PaddleServingClientExample e = new PaddleServingClientExample();
        boolean succ = false;
        
        if (args.length < 2) {
            System.out.println("Usage: java -cp <jar> PaddleServingClientExample <test-type> <configPath>.");
            System.out.println("<test-type>: http_proto http_json grpc bert cube_local yolov4 encrypt");
            return;
        }
        String testType = args[0];
        System.out.format("[Example] %s\n", testType);
        if ("http_proto".equals(testType)) {
            succ = e.http_proto(args[1]);
        } else if ("http_json".equals(testType)) {
            succ = e.http_json(args[1]);
        } else if ("grpc".equals(testType)) {
            succ = e.grpc(args[1]);
        } else if ("compress".equals(testType)) {
            succ = e.compress(args[1]);
        } else if ("bert".equals(testType)) {
            succ = e.bert(args[1]);
        } else if ("cube_local".equals(testType)) {
            succ = e.cube_local(args[1]);
        } else if ("yolov4".equals(testType)) {
            if (args.length < 3) {
                System.out.println("Usage: java -cp <jar> PaddleServingClientExample yolov4 <configPath> <image-filepath>.");
                return;
            }
            succ = e.yolov4(args[1],args[2]);
        } else if ("encrypt".equals(testType)) {
            if (args.length < 3) {
                System.out.println("Usage: java -cp <jar> PaddleServingClientExample encrypt <configPath> <keyPath>.");
                return;
            }
            succ = e.encrypt(args[1],args[2]);
        } else {
            System.out.format("test-type(%s) not match.\n", testType);
            return;
        }

        if (succ == true) {
            System.out.println("[Example] succ.");
        } else {
            System.out.println("[Example] fail.");
        }
    }
}
