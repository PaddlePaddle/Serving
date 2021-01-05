import io.paddle.serving.pipelineclient.*;
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

public class PipelineClientExample {

    boolean string_imdb_predict() {
        HashMap<String, String> feed_data
            = new HashMap<String, String>() {{
                put("words", "i am very sad | 0");
            }};
        System.out.println(feed_data);
        List<String> fetch = Arrays.asList("prediction");
        System.out.println(fetch);
        PipelineClient client = new PipelineClient();
        String target = "172.17.0.2:18070";
        boolean succ = client.connect(target);
        if (succ != true) {
            System.out.println("connect failed.");
            return false;
        }

        HashMap<String,String> result = client.predict(feed_data, fetch,false,0);
        if (result == null) {
            return false;
        }
        System.out.println(result);
        return true;
    }

    boolean asyn_predict() {
        HashMap<String, String> feed_data
            = new HashMap<String, String>() {{
                put("words", "i am very sad | 0");
            }};
        System.out.println(feed_data);
        List<String> fetch = Arrays.asList("prediction");
        System.out.println(fetch);
        PipelineClient client = new PipelineClient();
        String target = "172.17.0.2:18070";
        boolean succ = client.connect(target);
        if (succ != true) {
            System.out.println("connect failed.");
            return false;
        }
        PipelineFuture future = client.asyn_predict(feed_data, fetch,false,0);
        HashMap<String,String> result = future.get();
        if (result == null) {
            return false;
        }
        System.out.println(result);
        return true;
    }

    boolean indarray_predict() {
        float[] data = {0.0137f, -0.1136f, 0.2553f, -0.0692f, 0.0582f, -0.0727f, -0.1583f, -0.0584f, 0.6283f, 0.4919f, 0.1856f, 0.0795f, -0.0332f};
        INDArray npdata = Nd4j.createFromArray(data);

        HashMap<String, String> feed_data
            = new HashMap<String, String>() {{
                put("x", "array("+npdata.toString()+")");
            }};
        List<String> fetch = Arrays.asList("prediction");
        PipelineClient client = new PipelineClient();
        String target = "172.17.0.2:9998";
        boolean succ = client.connect(target);
        if (succ != true) {
            System.out.println("connect failed.");
            return false;
        }

        HashMap<String,String> result = client.predict(feed_data, fetch,false,0);
        if (result == null) {
            return false;
        }
        System.out.println(result);
        return true;
    }

    public static void main( String[] args ) {

        
        PipelineClientExample e = new PipelineClientExample();
        boolean succ = false;
        if (args.length < 1) {
            System.out.println("Usage: java -cp <jar> PaddleServingClientExample <test-type>.");
            System.out.println("<test-type>: fit_a_line bert model_ensemble asyn_predict batch_predict cube_local cube_quant yolov4");
            return;
        }
        String testType = args[0];
        System.out.format("[Example] %s\n", testType);
        if ("string_imdb_predict".equals(testType)) {
            succ = e.string_imdb_predict();
        }else if ("asyn_predict".equals(testType)) {
            succ = e.asyn_predict();
        }else if ("indarray_predict".equals(testType)) {
            succ = e.indarray_predict();
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
//if list or array or matrix,please Convert to INDArray,for example:
//INDArray npdata = Nd4j.createFromArray(data);

//INDArray Convert to String,for example:
//string value = "array("+npdata.toString()+")"

