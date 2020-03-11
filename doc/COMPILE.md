# 如何编译PaddleServing

### 编译环境设置
- os: CentOS 6u3
- gcc: 4.8.2及以上
- go: 1.9.2及以上
- git：2.17.1及以上
- cmake：3.2.2及以上
- python：2.7.2及以上

### 获取代码

``` python
git clone https://github.com/PaddlePaddle/Serving
git submodule update --init --recursive
```

### 编译Server部分

#### PYTHONROOT设置
``` shell
# 例如python的路径为/usr/bin/python，可以设置PYTHONROOT
export PYTHONROOT=/usr/
```

#### 集成CPU版本Paddle Inference Library
``` shell
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so -DPYTHON_EXECUTABLE=/home/users/dongdaxiang/software/baidu/third-party/python/bin/python -DCLIENT_ONLY=OFF ..
make -j10
```

#### 集成GPU版本Paddle Inference Library
``` shell
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so -DPYTHON_EXECUTABLE=/home/users/dongdaxiang/software/baidu/third-party/python/bin/python -DCLIENT_ONLY=ON -DWITH_GPU=ON ..
make -j10
```

### 编译Client部分

``` shell
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so -DPYTHON_EXECUTABLE=/home/users/dongdaxiang/software/baidu/third-party/python/bin/python -DCLIENT_ONLY=ON ..
make -j10
```

### 安装wheel包
无论是client端还是server端，编译完成后，安装python/dist/下的whl包即可
