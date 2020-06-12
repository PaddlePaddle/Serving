# OCR 

## Get Model
```
python -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
```

## RPC Service

### Start Service

```
python -m paddle_serving_server.serve --model ocr_rec_model --port 9292
```

### Client Prediction

```
python test_ocr_rec_client.py
```
