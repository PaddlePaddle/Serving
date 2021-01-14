wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing_example/encrypt.tar.gz
tar -xzf encrypt.tar.gz
cp -rvf ../fit_a_line/uci_housing_model .
cp -rvf ../fit_a_line/uci_housing_client .
