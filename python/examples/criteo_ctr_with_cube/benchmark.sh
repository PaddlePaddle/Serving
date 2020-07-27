rm profile_log
export FLAGS_profile_client=1
export FLAGS_profile_server=1

wget https://paddle-serving.bj.bcebos.com/unittest/ctr_cube_unittest.tar.gz --no-check-certificate
tar xf ctr_cube_unittest.tar.gz
mv models/ctr_client_conf ./
mv models/ctr_serving_model_kv ./
mv models/data ./cube/

wget https://paddle-serving.bj.bcebos.com/others/cube_app.tar.gz --no-check-certificate
tar xf cube_app.tar.gz
mv cube_app/cube* ./cube/
sh cube_prepare.sh &

python test_server.py ctr_serving_model_kv > serving_log 2>&1 &

for thread_num in 1 4 16
do
for batch_size in 1 4 16 64
do
    $PYTHONROOT/bin/python benchmark.py --thread $thread_num --batch_size $batch_size --model serving_client_conf/serving_client_conf.prototxt --request rpc > profile 2>&1
    echo "batch size : $batch_size"
    echo "thread num : $thread_num"
    echo "========================================"
    echo "batch size : $batch_size" >> profile_log
    $PYTHONROOT/bin/python ../util/show_profile.py profile $thread_num >> profile_log
    tail -n 3 profile >> profile_log
done
done

ps -ef|grep 'serving'|grep -v grep|cut -c 9-15 | xargs kill -9
