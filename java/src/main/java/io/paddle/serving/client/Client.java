package io.paddle.serving.client;
import java.util.*;
import java.util.function.Function;
import java.util.stream.*;
import java.util.Arrays;
import java.util.Iterator;
import java.lang.management.ManagementFactory;
import java.lang.management.RuntimeMXBean;
import java.util.stream.Collectors;
import java.util.stream.IntStream;
import java.util.stream.LongStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.Map.Entry;
import java.nio.file.*;
import org.nd4j.linalg.api.ndarray.INDArray;
import org.nd4j.linalg.api.iter.NdIndexIterator;
import org.nd4j.linalg.factory.Nd4j;
import org.nd4j.nativeblas.Nd4jCpu.boolean_and;

import java.lang.reflect.*;

import org.apache.http.HttpEntity;
import org.apache.http.NameValuePair;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.entity.StringEntity;
import org.apache.http.client.entity.GzipDecompressingEntity;
import org.apache.http.Header;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.util.EntityUtils;
import org.hamcrest.core.IsInstanceOf;
import org.apache.http.entity.InputStreamEntity;
import org.apache.http.entity.ByteArrayEntity;

import org.json.*;

import io.paddle.serving.configure.*;
import baidu.paddle_serving.predictor.general_model.*;

import org.apache.commons.lang3.ArrayUtils;


import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import io.grpc.StatusRuntimeException;
import com.google.protobuf.ByteString;


import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;
 

class ElementType {
    public static final int Int64_type = 0;
    public static final int Float32_type = 1;
    public static final int Int32_type = 2;
    public static final int String_type = 20;
    public static final Map<Integer, String> feedTypeToDataKey_;
    static
    {
        feedTypeToDataKey_ = new HashMap<Integer, String>();
        feedTypeToDataKey_.put(ElementType.Int64_type, "int64_data");
        feedTypeToDataKey_.put(ElementType.Float32_type, "float_data");
        feedTypeToDataKey_.put(ElementType.Int32_type, "int_data");
        feedTypeToDataKey_.put(ElementType.String_type, "data");
    }
}

class Profiler {
    int pid_;
    String print_head_ = null;
    List<String> time_record_ = null;
    boolean enable_ = false;

    Profiler() {
        RuntimeMXBean runtimeMXBean = ManagementFactory.getRuntimeMXBean();
        pid_ = Integer.valueOf(runtimeMXBean.getName().split("@")[0]).intValue();
        print_head_ = "\nPROFILE\tpid:" + pid_ + "\t";
        time_record_ = new ArrayList<String>();
        time_record_.add(print_head_);
    }

    void record(String name) {
        if (enable_) {
            long ctime = System.currentTimeMillis() * 1000;
            time_record_.add(name + ":" + String.valueOf(ctime) + " ");
        }
    }

    void printProfile() {
        if (enable_) {
            String profile_str = String.join("", time_record_);
            time_record_ = new ArrayList<String>();
            time_record_.add(print_head_);
        }
    }

    void enable(boolean flag) {
        enable_ = flag;
    }
}
public class Client {
    private int timeoutS_;
    private List<String> feedNames_;
    private Map<String, String> feedRealNames_;
    private Map<String, Integer> feedTypes_;
    private Map<String, List<Integer>> feedShapes_;
    private Map<String, Integer> feedNameToIndex_;
    private List<String> fetchNames_;
    private Map<String, Integer> fetchTypes_;
    private Set<String> lodTensorSet_;
    private Map<String, Integer> feedTensorLen_;
    private Profiler profiler_;
    private String ip;
    private String serverPort;
    private String port;
    private String serviceName;
    private boolean request_compress_flag;
    private boolean response_compress_flag;
    private String GLOG_v;
    private boolean http_proto;
    private boolean use_grpc_client;
    private ManagedChannel channel_;
    private GeneralModelServiceGrpc.GeneralModelServiceBlockingStub blockingStub_;


    public Client() {
        feedNames_ = null;
        feedRealNames_ = null;
        feedTypes_ = null;
        feedShapes_ = null;
        fetchNames_ = null;
        fetchTypes_ = null;
        lodTensorSet_ = null;
        feedTensorLen_ = null;
        feedNameToIndex_ = null;
        timeoutS_ = 200000;
        ip = "127.0.0.1";
        port = "9393";
        serverPort = "9393";
        serviceName = "/GeneralModelService/inference";
        request_compress_flag = false;
        response_compress_flag = false;
        GLOG_v = System.getenv("GLOG_v");
        http_proto = true;//use the Proto in HTTP by default.
        use_grpc_client = false;

        channel_ = null;
        blockingStub_ = null;

        profiler_ = new Profiler();
        boolean is_profile = false;
        String FLAGS_profile_client = System.getenv("FLAGS_profile_client");
        if (FLAGS_profile_client != null && FLAGS_profile_client.equals("1")) {
            is_profile = true;
        }
        profiler_.enable(is_profile);
    }

    public void setTimeOut(int timeoutS_) {
        this.timeoutS_ = timeoutS_;
    }

    public void setIP(String ip) {
        this.ip = ip;
    }

    public void setPort(String port) {
        this.port = port;
        this.serverPort = port;
    }

    public void setServiceName(String serviceName){
        this.serviceName = serviceName;
    }

    public void loadClientConfig(String model_config_path) {
        GeneralModelConfig.Builder model_conf_builder = GeneralModelConfig.newBuilder();
        try {
            byte[] data = Files.readAllBytes(Paths.get(model_config_path));
            String model_config_str = new String(data, "utf-8");
            com.google.protobuf.TextFormat.getParser().merge(model_config_str, model_conf_builder);
        } catch (com.google.protobuf.TextFormat.ParseException e) {
            System.out.format("Parse client config failed: %s\n", e.toString());
        } catch (Exception e) {
            System.out.format("Open client config failed: %s\n", e.toString());
        }
        GeneralModelConfig model_conf = model_conf_builder.build();

        feedNames_ = new ArrayList<String>();
        feedRealNames_ = new HashMap<String, String>();
        feedTypes_ = new HashMap<String, Integer>();
        feedShapes_ = new HashMap<String, List<Integer>>();
        lodTensorSet_ = new HashSet<String>();
        feedTensorLen_ = new HashMap<String, Integer>();
        feedNameToIndex_ = new HashMap<String, Integer>();

        fetchNames_ = new ArrayList<String>();
        fetchTypes_ = new HashMap<String, Integer>();

        List<FeedVar> feed_var_list = model_conf.getFeedVarList();
        for (int i = 0; i < feed_var_list.size(); ++i) {
            FeedVar feed_var = feed_var_list.get(i);
            String var_name = feed_var.getAliasName();
            feedNames_.add(var_name);
            feedRealNames_.put(var_name, feed_var.getName());
            feedTypes_.put(var_name, feed_var.getFeedType());
            feedShapes_.put(var_name, feed_var.getShapeList());
            feedNameToIndex_.put(var_name, i);
            if (feed_var.getIsLodTensor()) {
                lodTensorSet_.add(var_name);
            } else {
                int counter = 1;
                for (int dim : feedShapes_.get(var_name)) {
                    counter *= dim;
                }
                feedTensorLen_.put(var_name, counter);
            }
        }

        List<FetchVar> fetch_var_list = model_conf.getFetchVarList();
        for (int i = 0; i < fetch_var_list.size(); i++) {
            FetchVar fetch_var = fetch_var_list.get(i);
            String var_name = fetch_var.getAliasName();
            fetchNames_.add(var_name);
            fetchTypes_.put(var_name, fetch_var.getFetchType());
        }
    }

    public void use_key(String keyFilePath) {
        String key_str = null;
        String encrypt_url = "http://" + this.ip + ":" +this.port;
        try {
            byte[] data = Files.readAllBytes(Paths.get(keyFilePath));
            key_str = Base64.getEncoder().encodeToString(data);
        } catch (Exception e) {
            System.out.format("Open key file failed: %s\n", e.toString());
        }
        JSONObject jsonKey = new JSONObject();
        if( key_str != null) {
            jsonKey.put("key", key_str);
        }else{
            jsonKey.put("key", "");
        }
        String result = doPost(encrypt_url, jsonKey.toString());
        try {
            JSONObject jsonObject = new JSONObject(result);
            JSONArray jsonArray = jsonObject.getJSONArray("endpoint_list");
            this.serverPort = jsonArray.getString(0);
            System.out.format("Real ServerPort is: %s\n", this.serverPort);
        }catch (JSONException err) {
            System.out.format("Parse serverPort failed: %s\n", err.toString());
        }
    }

    public void set_request_compress(boolean request_compress_flag) {
        this.request_compress_flag = request_compress_flag;
    }

    public void set_response_compress(boolean response_compress_flag) {
        this.response_compress_flag = response_compress_flag;
    }

    public void set_http_proto(boolean http_proto){
        this.http_proto = http_proto;
    }
    public void set_use_grpc_client(boolean use_grpc_client){
        this.use_grpc_client = use_grpc_client;
    }

    public byte[] compress(Object obj) {
        if (obj == null) {
            return null;
        }
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        GZIPOutputStream gzip;
        try {
            gzip = new GZIPOutputStream(out);
            if(obj instanceof String){
                gzip.write(((String)obj).getBytes("UTF-8"));
            }else{
                gzip.write((byte[])obj);
            }
            gzip.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return out.toByteArray();
    }
    
    // 帮助用户封装Http请求的接口，用户只需要传递FeedData,Lod,Fetchlist即可。
    // 根据Proto组装Json的过程由这个函数来完成，且接口与Python基本一致.
    // 共提供了四组重载的接口，支持用户最少传入feedData和fetch，还可传lod和batchFlag.
    public String predict(Map<String, Object> feedData,
                    List<String> fetch,
                    int log_id) {
        
        return predict(feedData,null,fetch,false,log_id);
    }

    public String predict(Map<String, Object> feedData,
                    List<String> fetch,
                    boolean batchFlag,
                    int log_id) {
        
        return predict(feedData,null,fetch,batchFlag,log_id);
    }

    public String predict(Map<String, Object> feedData,
                    Map<String, Object> feedLod,
                    List<String> fetch,
                    int log_id) {
        
        return predict(feedData,feedLod,fetch,false,log_id);
    }

    public String predict(Map<String, Object> feedData,
                    Map<String, Object> feedLod,
                    List<String> fetch,
                    boolean batchFlag,
                    int log_id) {
        if(this.use_grpc_client){
            return grpc_predict(feedData, feedLod, fetch, batchFlag, log_id);
        }
        return http_predict(feedData, feedLod, fetch, batchFlag, log_id);
    }

    public String grpc_predict(Map<String, Object> feedData,
                    Map<String, Object> feedLod,
                    List<String> fetch,
                    boolean batchFlag,
                    int log_id) {
        String result = null;
        try {
            String server_url = this.ip + ":" + this.serverPort;
            channel_ = ManagedChannelBuilder.forTarget(server_url)
                .defaultLoadBalancingPolicy("round_robin")
                .maxInboundMessageSize(Integer.MAX_VALUE)
                .usePlaintext()
                .build();
            blockingStub_ = GeneralModelServiceGrpc.newBlockingStub(channel_);
            Request request = process_proto_data(feedData, feedLod, fetch, batchFlag, log_id);
            Response resp = blockingStub_.inference(request);
            result = resp.toString();
        } catch (Exception e) {
            System.out.format("grpc_predict failed: %s\n", e.toString());
            return null;
        }
        return result;
    }

    public String http_predict(Map<String, Object> feedData,
                    Map<String, Object> feedLod,
                    List<String> fetch,
                    boolean batchFlag,
                    int log_id) {
        String server_url = "http://" + this.ip + ":" + this.serverPort + this.serviceName;
        // 处理fetchList
        String result = null;
        if(this.http_proto){
            Request request = process_proto_data(feedData, feedLod, fetch, batchFlag, log_id);
            result = doPost(server_url, request.toByteArray());

        }else{
            JSONObject jsonRequest = process_json_data(feedData,feedLod,fetch,batchFlag,log_id);
            result = doPost(server_url, jsonRequest.toString());
        }
        return result;
    }

    public String doPost(String url, Object postData) {
        CloseableHttpClient httpClient = null;
        CloseableHttpResponse httpResponse = null;
        String result = "";
        // 创建httpClient实例
        httpClient = HttpClients.createDefault();
        // 创建httpPost远程连接实例
        HttpPost httpPost = new HttpPost(url);
        // 配置请求参数实例
        RequestConfig requestConfig = RequestConfig.custom().setConnectTimeout(timeoutS_)// 设置连接主机服务超时时间
                .setConnectionRequestTimeout(timeoutS_)// 设置连接请求超时时间
                .setSocketTimeout(timeoutS_)// 设置读取数据连接超时时间
                .build();
        // 为httpPost实例设置配置
        httpPost.setConfig(requestConfig);
        if(this.http_proto){
            httpPost.setHeader("Content-Type", "application/proto");
        }else{
            httpPost.setHeader("Content-Type", "application/json");
        }
        
        // 设置请求头
        if(response_compress_flag){
            httpPost.addHeader("Accept-encoding", "gzip");
            if(GLOG_v != null){
                System.out.format("------- Accept-encoding gzip:  \n");
            }
        }
        
        try {
            if(postData instanceof String){
                if(request_compress_flag && ((String)postData).length()>1024){
                    try{
                        byte[] gzipEncrypt = compress(postData);
                        httpPost.setEntity(new InputStreamEntity(new ByteArrayInputStream(gzipEncrypt), gzipEncrypt.length));
                        httpPost.addHeader("Content-Encoding", "gzip");
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }else{
                    httpPost.setEntity(new StringEntity((String)postData, "UTF-8"));
                }

            }else{
                if(request_compress_flag && ((byte[])postData).length>1024){
                    try{
                        byte[] gzipEncrypt = compress(postData);
                        httpPost.setEntity(new InputStreamEntity(new ByteArrayInputStream(gzipEncrypt), gzipEncrypt.length));
                        httpPost.addHeader("Content-Encoding", "gzip");
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }else{
                    httpPost.setEntity(new ByteArrayEntity((byte[])postData));
                    //httpPost.setEntity(new InputStreamEntity(new ByteArrayInputStream((byte[])postData), ((byte[])postData).length));
                }
            }
            
            // httpClient对象执行post请求,并返回响应参数对象
            httpResponse = httpClient.execute(httpPost);
            // 从响应对象中获取响应内容
            HttpEntity entity = httpResponse.getEntity();
            Header header = entity.getContentEncoding();
            if(GLOG_v != null){
                System.out.format("------- response header:  %s\n", header);
            }
            if(header != null && header.getValue().equalsIgnoreCase("gzip")){	//判断返回内容是否为gzip压缩格式
                GzipDecompressingEntity gzipEntity = new GzipDecompressingEntity(entity);
                result = EntityUtils.toString(gzipEntity);
                if(GLOG_v != null){
                    System.out.format("------- degzip response:  %s\n", result);
                }
            }else{
                if(this.http_proto){
                    
                    Response resp = Response.parseFrom(EntityUtils.toByteArray(entity));
                    result = resp.toString();
                }else{
                    result = EntityUtils.toString(entity);
                }
            }
        } catch (ClientProtocolException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            // 关闭资源
            if (null != httpResponse) {
                try {
                    httpResponse.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (null != httpClient) {
                try {
                    httpClient.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return result;
    }

    public List<Object> recursiveExtract(Object stuff) {

        List<Object> mylist = new ArrayList<Object>();
    
        if(stuff instanceof Iterable) {
            for(Object o : (Iterable< ? >)stuff) {
                mylist.addAll(recursiveExtract(o));
            }
        } else if(stuff instanceof Map) {
            for(Object o : ((Map<?, ? extends Object>) stuff).values()) {
                mylist.addAll(recursiveExtract(o));
            }
        } else {
            mylist.add(stuff);
        }
    
        return mylist;
    }

    public JSONObject process_json_data(Map<String, Object> feedData,
                Map<String, Object> feedLod,
                List<String> fetch,
                boolean batchFlag,
                int log_id){
        
        // 处理Tensor
        JSONArray jsonTensorArray = new JSONArray();
        try{
            if (null != feedData && feedData.size() > 0) {
                // 通过map集成entrySet方法获取entity
                Set<Entry<String, Object>> entrySet = feedData.entrySet();
                // 循环遍历，获取迭代器
                Iterator<Entry<String, Object>> iterator = entrySet.iterator();
                while (iterator.hasNext()) {
                    JSONObject jsonTensor = new JSONObject();
                    Entry<String, Object> mapEntry = iterator.next();
                    Object objectValue = mapEntry.getValue();
                    String feed_alias_name = mapEntry.getKey();
                    String feed_real_name = feedRealNames_.get(feed_alias_name);
                    List<Integer> shape = new ArrayList<Integer>(feedShapes_.get(feed_alias_name));
                    int element_type = feedTypes_.get(feed_alias_name);
                    
                    jsonTensor.put("alias_name", feed_alias_name);
                    jsonTensor.put("name", feed_real_name);
                    jsonTensor.put("elem_type", element_type);

                    // 处理数据与shape
                    String protoDataKey = ElementType.feedTypeToDataKey_.get(element_type);
                    // 如果是INDArray类型，先转为一维.
                    // 此时shape为INDArray的shape
                    if(objectValue instanceof INDArray){
                        INDArray tempIndArray = (INDArray)objectValue;
                        long[] indarrayShape = tempIndArray.shape();
                        shape.clear();
                        for(long dim:indarrayShape){
                            shape.add((int)dim);
                        }
                        if(element_type == ElementType.Int64_type){
                            objectValue = tempIndArray.data().asLong();
                        }else if(element_type == ElementType.Int32_type){
                            objectValue = tempIndArray.data().asInt();
                        }else if(element_type == ElementType.Float32_type){
                            objectValue = tempIndArray.data().asFloat();
                        }else{
                            throw new Exception("INDArray 类型不支持");
                        }
                    }else if(objectValue.getClass().isArray()){
                        // 如果是数组类型，则无须处理，直接使用即可。
                        // 且数组无法嵌套，此时batch无法从数据中获取
                        // 默认batch维度为1，或者feedVar的shape信息中已包含batch
                    }else if(objectValue instanceof List){
                        // 如果为list，可能存在嵌套，此时需要展平
                        // 如果batchFlag为True，则认为是嵌套list
                        // 此时取最外层为batch的维度
                        if (batchFlag) {
                            List<?> list = new ArrayList<>();
                            list = new ArrayList<>((Collection<?>)objectValue);
                            // 在index=0处，加上batch
                            shape.add(0, list.size());
                        }
                        objectValue = recursiveExtract(objectValue);
                    }else{
                        // 此时认为是传入的单个String或者Int等
                        // 此时无法获取batch信息，故对shape不处理
                        // 由于Proto中为Repeated,需要把数据包装成list
                        if(objectValue instanceof String){
                            if(feedTypes_.get(protoDataKey)!= ElementType.String_type){
                                throw new Exception("feedvar is not string-type,feed can`t be a single string.");
                            }
                        }else{
                            if(feedTypes_.get(protoDataKey)== ElementType.String_type){
                                throw new Exception("feedvar is string-type,feed, feed can`t be a single int or others.");
                            }
                        }
                        List<Object> list = new ArrayList<>();
                        list.add(objectValue);
                        objectValue = list;
                    }
                    jsonTensor.put(protoDataKey,objectValue);
                    if(!batchFlag){
                        // 在index=0处，加上batch=1
                        shape.add(0, 1);
                    }
                    jsonTensor.put("shape", shape);
                    
                    // 处理lod信息，支持INDArray Array Iterable
                    Object feedLodValue = null;
                    if(feedLod != null){
                        feedLodValue = feedLod.get(feed_alias_name);
                        if(feedLodValue != null) {
                            if(feedLodValue instanceof INDArray){
                                INDArray tempIndArray = (INDArray)feedLodValue;    
                                feedLodValue = tempIndArray.data().asInt();
                            }else if(feedLodValue.getClass().isArray()){
                                // 如果是数组类型，则无须处理，直接使用即可。
                            }else if(feedLodValue instanceof Iterable){
                                // 如果为list，可能存在嵌套，此时需要展平
                                feedLodValue = recursiveExtract(feedLodValue);
                            }else{
                                throw new Exception("Lod must be INDArray or Array or Iterable.");
                            }
                            jsonTensor.put("lod", feedLodValue);
                        }
                    }
                    jsonTensorArray.put(jsonTensor);
                }
            }
        }catch (Exception e) {
            e.printStackTrace();
        }

        JSONArray jsonFetchList = new JSONArray(fetch);
        /*
        Iterator<String> fetchIterator = fetch.iterator();
        while (fetchIterator.hasNext()) {
            jsonFetchList.put(fetchIterator.next());
        }
        */
        JSONObject jsonRequest = new JSONObject();
        jsonRequest.put("log_id",log_id);
        jsonRequest.put("fetch_var_names", jsonFetchList);
        jsonRequest.put("tensor",jsonTensorArray);
        if(GLOG_v != null){
            System.out.format("------- Final jsonRequest:  %s\n", jsonRequest.toString());
        }
        return jsonRequest;
    }

    public Request process_proto_data(Map<String, Object> feedData,
                Map<String, Object> feedLod,
                List<String> fetch,
                boolean batchFlag,
                int log_id){
        // 处理Tensor
        Request.Builder request_builder = Request.newBuilder().addAllFetchVarNames(fetch).setLogId(log_id);
        try{
            if (null != feedData && feedData.size() > 0) {
                // 通过map集成entrySet方法获取entity
                Set<Entry<String, Object>> entrySet = feedData.entrySet();
                // 循环遍历，获取迭代器
                Iterator<Entry<String, Object>> iterator = entrySet.iterator();
                while (iterator.hasNext()) {
                    Tensor.Builder tensor_builder = Tensor.newBuilder();
                    Entry<String, Object> mapEntry = iterator.next();
                    Object objectValue = mapEntry.getValue();
                    String feed_alias_name = mapEntry.getKey();
                    String feed_real_name = feedRealNames_.get(feed_alias_name);
                    List<Integer> shape = new ArrayList<Integer>(feedShapes_.get(feed_alias_name));
                    int element_type = feedTypes_.get(feed_alias_name);
                    
                    tensor_builder.setAliasName(feed_alias_name);
                    tensor_builder.setName(feed_real_name);
                    tensor_builder.setElemType(element_type);

                    // 处理数据与shape
                    // 如果是INDArray类型，先转为一维.
                    // 此时shape为INDArray的shape
                    if(objectValue instanceof INDArray){
                        INDArray tempIndArray = (INDArray)objectValue;
                        long[] indarrayShape = tempIndArray.shape();
                        shape.clear();
                        for(long dim:indarrayShape){
                            shape.add((int)dim);
                        }   
                        if(element_type == ElementType.Int64_type){
                            
                            List<Long> iter = Arrays.stream(tempIndArray.data().asLong()).boxed().collect(Collectors.toList());
                            tensor_builder.addAllInt64Data(iter);
                            
                        }else if(element_type == ElementType.Int32_type){
                            
                            List<Integer> iter = Arrays.stream(tempIndArray.data().asInt()).boxed().collect(Collectors.toList());
                            tensor_builder.addAllIntData(iter);
                            
                        }else if(element_type == ElementType.Float32_type){
                            List<Float> iter = Arrays.asList(ArrayUtils.toObject(tempIndArray.data().asFloat()));
                            tensor_builder.addAllFloatData(iter);
                            
                        }else{
                            // 看接口是String还是Bytes
                            throw new Exception("INDArray 类型不支持");
                        }
                    }else if(objectValue.getClass().isArray()){
                        // 如果是数组类型，则无须处理，直接使用即可。
                        // 且数组无法嵌套，此时batch无法从数据中获取
                        // 默认batch维度为1，或者feedVar的shape信息中已包含batch
                        if(element_type == ElementType.Int64_type){
                            List<Long> iter = Arrays.stream((long[])objectValue).boxed().collect(Collectors.toList());
                            tensor_builder.addAllInt64Data(iter);
                        }else if(element_type == ElementType.Int32_type){
                            List<Integer> iter = Arrays.stream((int[])objectValue).boxed().collect(Collectors.toList());
                            tensor_builder.addAllIntData(iter);
                        }else if(element_type == ElementType.Float32_type){
                            List<Float> iter = Arrays.asList(ArrayUtils.toObject((float[])objectValue));
                            tensor_builder.addAllFloatData(iter);
                        }else{
                            List<String> iter = Arrays.asList((String[])objectValue);
                            tensor_builder.addAllData(iter);
                        }
                    }else if(objectValue instanceof List){
                        // 如果为list，可能存在嵌套，此时需要展平
                        // 如果batchFlag为True，则认为是嵌套list
                        // 此时取最外层为batch的维度
                        if (batchFlag) {
                            List<?> list = new ArrayList<>();
                            list = new ArrayList<>((Collection<?>)objectValue);
                            // 在index=0处，加上batch
                            shape.add(0, list.size());
                        }
                        if(element_type == ElementType.Int64_type){
                            tensor_builder.addAllInt64Data((List<Long>)(List)recursiveExtract(objectValue));
                        }else if(element_type == ElementType.Int32_type){
                            tensor_builder.addAllIntData((List<Integer>)(List)recursiveExtract(objectValue));
                        }else if(element_type == ElementType.Float32_type){
                            tensor_builder.addAllFloatData((List<Float>)(List)recursiveExtract(objectValue));
                        }else{
                            // 看接口是String还是Bytes
                            tensor_builder.addAllData((List<String>)(List)recursiveExtract(objectValue));
                        }
                    }else{
                        // 此时认为是传入的单个String或者Int等
                        // 此时无法获取batch信息，故对shape不处理
                        // 由于Proto中为Repeated,需要把数据包装成list
                        List<Object> tempList = new ArrayList<>();
                        tempList.add(objectValue);
                        if(element_type == ElementType.Int64_type){
                            tensor_builder.addAllInt64Data((List<Long>)(List)tempList);
                        }else if(element_type == ElementType.Int32_type){
                            tensor_builder.addAllIntData((List<Integer>)(List)tempList);
                        }else if(element_type == ElementType.Float32_type){
                            tensor_builder.addAllFloatData((List<Float>)(List)tempList);
                        }else{
                            // 看接口是String还是Bytes
                            tensor_builder.addAllData((List<String>)(List)tempList);
                        }
                    }
                    if(!batchFlag){
                        // 在index=0处，加上batch=1
                        shape.add(0, 1);
                    }
                    tensor_builder.addAllShape(shape);
                    
                    // 处理lod信息，支持INDArray Array Iterable
                    Object feedLodValue = null;
                    if(feedLod != null){
                        feedLodValue = feedLod.get(feed_alias_name);
                        if(feedLodValue != null) {
                            if(feedLodValue instanceof INDArray){
                                INDArray tempIndArray = (INDArray)feedLodValue;
                                List<Integer> iter = Arrays.stream(tempIndArray.data().asInt()).boxed().collect(Collectors.toList());
                                tensor_builder.addAllLod(iter);
                            }else if(feedLodValue.getClass().isArray()){
                                // 如果是数组类型，则无须处理，直接使用即可。
                                List<Integer> iter = Arrays.stream((int[])feedLodValue).boxed().collect(Collectors.toList());
                                tensor_builder.addAllLod(iter);
                            }else if(feedLodValue instanceof Iterable){
                                // 如果为list，可能存在嵌套，此时需要展平
                                tensor_builder.addAllLod((List<Integer>)(List)recursiveExtract(feedLodValue));
                            }else{
                                throw new Exception("Lod must be INDArray or Array or Iterable.");
                            }
                        }
                    }
                    request_builder.addTensor(tensor_builder.build());
                }
            }
        }catch (Exception e) {
            e.printStackTrace();
        }
        return request_builder.build();
    }


}

