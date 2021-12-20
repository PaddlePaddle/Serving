rm -rf log
rm -rf workdir*
export GLOG_v=3
nohup python3 -m  paddle_serving_server.serve --model picodet_PPLCNet_x2_5_mainbody_lite_v2.0_serving general_PPLCNet_x2_5_lite_v2.0_serving  --op GeneralPicodetOp GeneralFeatureExtractOp --port 9400 &
