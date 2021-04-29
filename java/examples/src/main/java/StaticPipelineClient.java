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
* static resource management class
* @author HexToString
*/
public class StaticPipelineClient {
    /**
     * Static Variable PipelineClient
     */
    public static PipelineClient client = new PipelineClient();
    /**
     * the sign of connect status
     */
    public static boolean succ = false;

    /**
   * This method returns the sign of connect status.
   * @param strIp String type(The server ipv4) such as "192.168.10.10".
   * @param strPort String type(The server port) such as "8891".
   * @return boolean (the sign of connect status).
   */
    public static boolean initClient(String strIp,String strPort) {
        String target = strIp+ ":"+ strPort;//"172.17.0.2:18070";
        System.out.println("initial connect.");
        if (succ) {
            System.out.println("already connect.");
            return true;
        }
        succ = client.connect(target);
        if (succ != true) {
            System.out.println("connect failed.");
            return false;
        }
        return true;
    }
}

