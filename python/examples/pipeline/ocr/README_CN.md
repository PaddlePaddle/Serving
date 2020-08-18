# OCR 服务

([English](./README.md)|简体中文)

## 获取模型
```
python -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
```
## 获取数据集（可选）
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/ocr/test_imgs.tar
tar xf test_imgs.tar
```

## 启动服务

```
python -m paddle_serving_server.serve --model ocr_det_model --port 12000 --gpu_id 0 &> det.log &
python -m paddle_serving_server.serve --model ocr_rec_model --port 12001 --gpu_id 0 &> rec.log &
python pipeline_server.py &>pipeline.log &
```

## 启动客户端
```
python pipeline_client.py
```
