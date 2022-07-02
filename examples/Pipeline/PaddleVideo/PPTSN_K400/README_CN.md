# PP-TSN 视频分类模型

## 模型简介
PP-TSN模型是对TSN模型进行改进，在不增加参数量和计算量的情况下，得到了的更高精度的2D实用视频分类模型。

## 获取模型
```
sh get_model.sh
```

## 部署模型并测试

### 启动 server
```
python3 web_service.py &>log.txt &
```

## 启动 client

**一. http client**
```
python3 pipeline_http_client.py
```
**二. rpc client**

```
python3 pipeline_rpc_client.py
```
