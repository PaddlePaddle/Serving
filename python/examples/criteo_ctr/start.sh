cd fileserver
nohup python -m SimpleHTTPServer 8080 > fs.log &
cd ..
cd cube/
nohup ./cube > nohup_cube.log &
nohup ./cube-agent -P 8001 > nohup_agent.log &
sleep 1
nohup ./cube-transfer -p 8099 -l 4 --config conf/transfer.conf > nohup_transfer.log &
cd /
nohup python -m SimpleHTTPServer 8098 > fs.log &
cd -
sleep 30
cd cube
./cube-cli -config_file conf/cube.conf  -keys keys -dict test_dict -print_output true -batch 1
cd ..
mkdir work_dir1
yes | cp cube/conf/cube.conf ./work_dir1/
python test_server.py ctr_serving_model_kv &
python test_client.py ctr_client_conf/serving_client_conf.prototxt ./raw_data &


