
mkdir -p cube_model
mkdir -p cube/data
./seq_generator ctr_serving_model/SparseFeatFactors ./cube_model/feature
./cube/cube-builder -dict_name=test_dict -job_mode=base -last_version=0 -cur_version=0 -depend_version=0 -input_path=./cube_model -output_path=./cube/data -shard_num=1  -only_build=false
mv ./cube/data/0_0/test_dict_part0/* ./cube/data/
cd cube && ./cube 
