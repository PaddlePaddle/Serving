# IMDB model ensemble examples

## Get models
```
sh get_data.sh
```

## Start servers

```
python3 -m paddle_serving_server.serve --model imdb_cnn_model --port 9292 &> cnn.log &
python3 -m paddle_serving_server.serve --model imdb_bow_model --port 9393 &> bow.log &
python3 test_pipeline_server.py &>pipeline.log &
```

## Start clients
```
python3 test_pipeline_client.py
```
