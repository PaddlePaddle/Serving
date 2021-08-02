package io.paddle.serving.client;
import java.util.*;
import java.util.function.Function;
import java.lang.management.ManagementFactory;
import java.lang.management.RuntimeMXBean;
import java.util.stream.Collectors;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.Map.Entry;
import java.nio.file.*;
import org.nd4j.linalg.api.ndarray.INDArray;
import org.nd4j.linalg.api.iter.NdIndexIterator;
import org.nd4j.linalg.factory.Nd4j;
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

import org.json.*;

import io.paddle.serving.configure.*;


import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;
 

enum ElementType
{
    Int64_type, Float32_type, Int32_type, Bytes_type;
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
public class HttpClient {
    private int httpTimeoutS_;
    private List<String> feedNames_;
    private Map<String, String> feedRealNames_;
    private Map<String, Integer> feedTypes_;
    private Map<String, List<Integer>> feedShapes_;
    private Map<String, Integer> feedNameToIndex_;
    private Map<Integer, String> feedTypeToDataKey_;
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

    public HttpClient() {
        feedNames_ = null;
        feedRealNames_ = null;
        feedTypes_ = null;
        feedShapes_ = null;
        fetchNames_ = null;
        fetchTypes_ = null;
        lodTensorSet_ = null;
        feedTensorLen_ = null;
        feedNameToIndex_ = null;
        httpTimeoutS_ = 200000;
        ip = "127.0.0.1";
        port = "9393";
        serverPort = "9393";
        serviceName = "/GeneralModelService/inference";
        request_compress_flag = false;
        response_compress_flag = false;

        feedTypeToDataKey_ = new HashMap<Integer, String>();
        feedTypeToDataKey_.put(0, "int64_data");
        feedTypeToDataKey_.put(1, "float_data");
        feedTypeToDataKey_.put(2, "int_data");
        feedTypeToDataKey_.put(3, "data");

        profiler_ = new Profiler();
        boolean is_profile = false;
        String FLAGS_profile_client = System.getenv("FLAGS_profile_client");
        if (FLAGS_profile_client != null && FLAGS_profile_client.equals("1")) {
            is_profile = true;
        }
        profiler_.enable(is_profile);
    }

    public void setTimeOut(int httpTimeoutS_) {
        this.httpTimeoutS_ = httpTimeoutS_;
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
            String model_config_str = Files.readString(Paths.get(model_config_path));
            com.google.protobuf.TextFormat.getParser().merge(model_config_str, model_conf_builder);
        } catch (com.google.protobuf.TextFormat.ParseException e) {
            System.out.format("Parse client config failed: %s\n", e.toString());
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
            key_str = Files.readString(Paths.get(keyFilePath));
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
        // need to be done.
        this.request_compress_flag = request_compress_flag;
    }

    public void set_response_compress(boolean response_compress_flag) {
        // need to be done.
        this.response_compress_flag = response_compress_flag;
    }

    public static String compress(String str,String inEncoding) throws IOException {
        if (str == null || str.length() == 0) {
          return str;
        }
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        GZIPOutputStream gzip = new GZIPOutputStream(out);
        gzip.write(str.getBytes(inEncoding));
        gzip.close();
        return out.toString("ISO-8859-1");
        
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
                        String server_url = "http://" + this.ip + ":" + this.serverPort + this.serviceName;
                        // 处理fetchList
                        JSONArray jsonFetchList = new JSONArray();
                        Iterator<String> fetchIterator = fetch.iterator();
                        while (fetchIterator.hasNext()) {
                            jsonFetchList.put(fetchIterator.next());
                        }

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
                                    List<Integer> shape = feedShapes_.get(feed_alias_name);
                                    int element_type = feedTypes_.get(feed_alias_name);
                                    String protoDataKey = feedTypeToDataKey_.get(element_type);
                                    Object feedLodValue = feedLod.get(feed_alias_name);
                                    // 如果是INDArray类型，先转为一维，再objectValue.ToString.
                                    // 如果是String或List，则直接objectValue.ToString.
                                    if(objectValue.getClass().equals(INDArray.class)){
                                        long[] flattened_shape = {-1};
                                        Class<?> classLongArray = flattened_shape.getClass();
                                        Method methodReshape = mapEntry.getValue().getClass().getMethod("reshape", classLongArray);
                                        Method methodShape = mapEntry.getValue().getClass().getMethod("shape");
                                        long[] indarrayShape = (long[])methodShape.invoke(objectValue);
                                        shape.clear();
                                        for(long dim:indarrayShape){
                                            shape.add((int)dim);
                                        }
                                        objectValue = methodReshape.invoke(objectValue,flattened_shape);
                                    }
                                    if(batchFlag){
                                        // 在index=0处，加上batch=1
                                        shape.add(0, 1);
                                    }
                                    jsonTensor.put("alias_name", feed_alias_name);
                                    jsonTensor.put("name", feed_real_name);
                                    jsonTensor.put("shape", shape);
                                    
                                    jsonTensor.put("elem_type", element_type);
                                    jsonTensor.put(protoDataKey,objectValue);
                                    if(feedLodValue != null) {
                                        jsonTensor.put("lod", feedLodValue);
                                    }
                                    jsonTensorArray.put(jsonTensor);
                                }
                            }
                        }catch (Exception e) {
                            e.printStackTrace();
                        }
                        JSONObject jsonRequest = new JSONObject();
                        jsonRequest.put("log_id",log_id);
                        jsonRequest.put("fetch_var_names", jsonFetchList);
                        jsonRequest.put("tensor",jsonTensorArray);
                        return doPost(server_url, jsonRequest.toString());
                    }

    public String doPost(String url, String strPostData) {
        CloseableHttpClient httpClient = null;
        CloseableHttpResponse httpResponse = null;
        String result = "";
        // 创建httpClient实例
        httpClient = HttpClients.createDefault();
        // 创建httpPost远程连接实例
        HttpPost httpPost = new HttpPost(url);
        // 配置请求参数实例
        RequestConfig requestConfig = RequestConfig.custom().setConnectTimeout(httpTimeoutS_)// 设置连接主机服务超时时间
                .setConnectionRequestTimeout(httpTimeoutS_)// 设置连接请求超时时间
                .setSocketTimeout(httpTimeoutS_)// 设置读取数据连接超时时间
                .build();
        // 为httpPost实例设置配置
        httpPost.setConfig(requestConfig);
        // 设置请求头
        httpPost.addHeader("Content-Type", "application/json");
        if(response_compress_flag){
            httpPost.addHeader("Accept-encoding", "gzip");
        }
        if(request_compress_flag && strPostData.length()>512){
            try{
                strPostData = compress(strPostData,"UTF-8");
                httpPost.addHeader("Content-Encoding", "gzip");
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        try {
            httpPost.setEntity(new StringEntity(strPostData, "UTF-8"));
            // httpClient对象执行post请求,并返回响应参数对象
            httpResponse = httpClient.execute(httpPost);
            // 从响应对象中获取响应内容
            HttpEntity entity = httpResponse.getEntity();
            Header header = entity.getContentEncoding();
            if(header != null && header.getValue().equalsIgnoreCase("gzip")){	//判断返回内容是否为gzip压缩格式
                GzipDecompressingEntity gzipEntity = new GzipDecompressingEntity(entity);
                result = EntityUtils.toString(gzipEntity);
            }else{
                result = EntityUtils.toString(entity);
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
}

