# Fit a line example, prediction through rpc service
Start rpc service
``` shell
sh get_data.sh
python test_server.py uci_housing_model/
```

Prediction
``` shell
python test_client.py uci_housing_client/serving_client_conf.prototxt
```

# prediction through http service
Start a web service with default web service hosting modules
``` shell
python -m paddle_serving_server.web_serve --model uci_housing_model/ --thread 10 --name uci --port 9393
```

Prediction through http post
``` shell
curl -H "Content-Type:application/json" -X POST -d '{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332], "fetch":["price"]}' http://127.0.0.1:9393/uci/prediction
```