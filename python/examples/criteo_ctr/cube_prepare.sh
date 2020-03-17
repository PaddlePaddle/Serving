
mkdir -p cube_model
mkdir -p cube/data
./seq_generator ctr_serving_model/SparseFeatFactors ./cube_model/feature
/Serving/python/examples/criteo_ctr/cube/cube-builder -dict_name=test_dict -job_mode=base -last_version=0 -cur_version=0 -depend_version=0 -input_path=/Serving/python/examples/criteo_ctr/cube_model -output_path=/Serving/python/examples/criteo_ctr/cube/data -shard_num=1  -only_build=false
mv /Serving/python/examples/criteo_ctr/cube/data/0_0/test_dict_part0/* /Serving/python/examples/criteo_ctr/cube/data/
cd cube && ./cube 

