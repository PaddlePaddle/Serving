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

/**
* this class give an example for using the client to predict(grpc)
* StaticPipelineClient.client supports mutil-thread.
* By setting StaticPipelineClient.client properties，you can change the Maximum concurrency
* Do not need to generate multiple instances of client,Use the StaticPipelineClient.client or SingleTon instead.
* @author HexToString
*/
public class PipelineClientExample {

    /**
   * This method gives an example of synchronous prediction whose input type is string.
   */
    boolean string_imdb_predict() {
        HashMap<String, String> feed_data
            = new HashMap<String, String>() {{
                put("words", "i am very sad | 0");
            }};
        System.out.println(feed_data);
        List<String> fetch = Arrays.asList("prediction");
        System.out.println(fetch);
        
        if (StaticPipelineClient.succ != true) {
            if (!StaticPipelineClient.initClient("127.0.0.1","18070")) {
                System.out.println("connect failed.");
                return false;
            }
        }
        HashMap<String,String> result = StaticPipelineClient.client.predict(feed_data, fetch,false,0);
        if (result == null) {
            return false;
        }
        System.out.println(result);
        return true;
    }

    /**
   * This method gives an example of asynchronous prediction whose input type is string.
   */
    boolean asyn_predict() {
        HashMap<String, String> feed_data
            = new HashMap<String, String>() {{
                put("words", "i am very sad | 0");
            }};
        System.out.println(feed_data);
        List<String> fetch = Arrays.asList("prediction");
        System.out.println(fetch);
        if (StaticPipelineClient.succ != true) {
            if (!StaticPipelineClient.initClient("127.0.0.1","18070")) {
                System.out.println("connect failed.");
                return false;
            }
        }
        PipelineFuture future = StaticPipelineClient.client.asyn_predict(feed_data, fetch,false,0);
        HashMap<String,String> result = future.get();
        if (result == null) {
            return false;
        }
        System.out.println(result);
        return true;
    }

    /**
   * This method gives an example of synchronous prediction whose input type is Array or list or matrix.
   * use Nd4j.createFromArray method to convert Array to INDArray.
   * use convertINDArrayToString method to convert INDArray to specified String type(for python Numpy eval method).
   */
    boolean indarray_predict() {
        float[] data = {0.0137f, -0.1136f, 0.2553f, -0.0692f, 0.0582f, -0.0727f, -0.1583f, -0.0584f, 0.6283f, 0.4919f, 0.1856f, 0.0795f, -0.0332f};
        INDArray npdata = Nd4j.createFromArray(data);

        HashMap<String, String> feed_data
            = new HashMap<String, String>() {{
                put("x", convertINDArrayToString(npdata));
            }};
        List<String> fetch = Arrays.asList("prediction");
        if (StaticPipelineClient.succ != true) {
            if (!StaticPipelineClient.initClient("127.0.0.1","9998")) {
                System.out.println("connect failed.");
                return false;
            }
        }

        HashMap<String,String> result = StaticPipelineClient.client.predict(feed_data, fetch,false,0);
        if (result == null) {
            return false;
        }
        System.out.println(result);
        return true;
    }

    /**
   * This method convert INDArray to specified String type.
   * @param npdata INDArray type(The input data).
   * @return String (specified String type for python Numpy eval method).
   */
    String convertINDArrayToString(INDArray npdata) {
        return "array("+npdata.toString()+")";
    }

    /**
   * This method is entry function.
   * @param args String[] type(Command line parameters)
   */
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


