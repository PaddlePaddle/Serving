rm profile_log

#wget https://paddle-serving.bj.bcebos.com/unittest/ctr_cube_unittest.tar.gz --no-check-certificate
#tar xf ctr_cube_unittest.tar.gz
mv models/ctr_client_conf ./
mv models/ctr_serving_model_kv ./
mv models/data ./cube/

#wget https://paddle-serving.bj.bcebos.com/others/cube_app.tar.gz --no-check-certificate
#tar xf cube_app.tar.gz
mv cube_app/cube* ./cube/
sh cube_prepare.sh &

cp ../../../build_server/core/cube/cube-api/cube-cli .
python gen_key.py

for thread_num in 1 4 16 32
do
for batch_size in 1000
do
    ./cube-cli -config_file ./cube/conf/cube.conf -keys key -dict test_dict -thread_num $thread_num --batch $batch_size > profile 2>&1
    echo "batch size : $batch_size"
    echo "thread num : $thread_num"
    echo "========================================"
    echo "batch size : $batch_size" >> profile_log
    echo "thread num : $thread_num" >> profile_log
    tail -n 8 profile >> profile_log

done
done

ps -ef|grep 'cube'|grep -v grep|cut -c 9-15 | xargs kill -9
