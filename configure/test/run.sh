
echo ====RUN ./test_conf====
./test_conf > ./test_conf.log
diff ./test_conf.res ./test_conf.log >> res


echo ====RUN ./test_demo====
./test_demo > ./test_demo.log
diff ./test_demo.res ./test_demo.log >> res


echo ====RUN ./test_err====
./test_err > ./test_err.log
diff ./test_err.res ./test_err.log >> res


echo ====RUN ./test_get_value====
./test_get_value > ./test_get_value.log
diff ./test_get_value.res ./test_get_value.log >> res


echo ====RUN ./test_option====
./test_option > ./test_option.log
diff ./test_option.res ./test_option.log >> res


echo ====RUN ./test_rawdata====
./test_rawdata > ./test_rawdata.log
diff ./test_rawdata.res ./test_rawdata.log >> res


echo ====RUN ./test_reader====
./test_reader > ./test_reader.log
diff ./test_reader.res ./test_reader.log >> res


echo ====RUN ./test_rebuild====
./test_rebuild > ./test_rebuild.log
diff ./test_rebuild.res ./test_rebuild.log >> res


echo ====RUN ./test_str====
./test_str > ./test_str.log
diff ./test_str.res ./test_str.log >> res


echo ====RUN ./test_t1====
./test_t1 > ./test_t1.log
diff ./test_t1.res ./test_t1.log >> res


echo ====RUN ./test_trans_float====
./test_trans_float > ./test_trans_float.log
diff ./test_trans_float.res ./test_trans_float.log >> res


echo ====RUN ./test_trans_str2str====
./test_trans_str2str > ./test_trans_str2str.log
diff ./test_trans_str2str.res ./test_trans_str2str.log >> res


echo ====RUN ./test_xiaowei====
./test_xiaowei > ./test_xiaowei.log
diff ./test_xiaowei.res ./test_xiaowei.log >> res

echo ======= Done! ==============
echo 
echo | awk '{print "\033[1;32mIf *.log is diff to *.res, the diff is written to \033[31mres\033[m"}'
echo 
