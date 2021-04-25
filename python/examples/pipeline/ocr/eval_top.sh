sed -e "s/imagenet/${modelname}/g" benchmark_config.yaml.template >benchmark_config.yaml
sh benchmark.sh
mv std_benchmark.log std_benchmark.log.cpu
sed -e "s/use_gpu=0/use_gpu=1/g" benchmark.sh>benchmark_gpu.sh
sed -e "s/imagenet/${modelname}/g" -e "s/runtime_device: \"cpu\"/runtime_device: \"gpu\"/g" benchmark_config.yaml.template >benchmark_config.yaml
sh benchmark_gpu.sh
mv std_benchmark.log std_benchmark.log.gpu
cp std_benchmark.log.cpu /paddle/Cls/tools/serving_log/${modelname}.log.cpu
cp std_benchmark.log.gpu /paddle/Cls/tools/serving_log/${modelname}.log.gpu
