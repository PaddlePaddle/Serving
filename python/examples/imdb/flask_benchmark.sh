# !/bin/bash

GLOG_v=2 $PYTHONROOT/bin/python test_server.py serving_server_model/ 9292 >serving.std 2>&1 &
$PYTHONROOT/bin/python imdb_flask.py ~/serving_2020/http_client/Serving/python/examples/imdb/serving_server_model/ > flask_webserve.log 2>&1 &

for i in {1,2,4,8,12,16,20,24}
do
    $PYTHONROOT/bin/python flask_client_multithread.py serving_client_conf/serving_client_conf.prototxt test_data/flask_data $i > flask_cli.profile 2>flask_cli.err 
    tail -1 flask.profile
    cat flask_cli.profile
done
