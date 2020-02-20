# 如何编译PaddleServing

### 获取代码

``` python
git clone https://github.com/PaddlePaddle/Serving
git submodule update --init --recursive
```

### 编译Server部分

``` shell
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so -DPYTHON_EXECUTABLE=/home/users/dongdaxiang/software/baidu/third-party/python/bin/python -DCLIENT_ONLY=OFF ..
make -j10
```

### 编译Client部分

``` shell
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so -DPYTHON_EXECUTABLE=/home/users/dongdaxiang/software/baidu/third-party/python/bin/python -DCLIENT_ONLY=ON ..
make -j10
```

