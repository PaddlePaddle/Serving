sleep 5
cd cube
./cube-cli -config_file conf/cube.conf  -keys keys -dict test_dict -print_output true -batch 1
cd ..
mkdir work_dir1
yes | cp cube/conf/cube.conf ./work_dir1/
python test_server.py ctr_serving_model_kv &
python test_client.py ctr_client_conf/serving_client_conf.prototxt ./raw_data &
