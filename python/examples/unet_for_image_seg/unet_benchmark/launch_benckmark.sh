#!/bin/bash
python unet_benchmark.py --thread 1 --batch_size 1 --model ../unet_client/serving_client_conf.prototxt
# thread/batch can be modified as you wish 
