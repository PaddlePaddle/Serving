set -e
set -v

version=0.1.4

cd ./python
python change_version.py $version
cd ..

export PYTHONROOT=/usr/
export CUDA_CUDART_LIBRARY=/usr/local/cuda/lib64/libcudart.so

git fetch upstream
git merge upstream/v0.2.0

git submodule init
git submodule update

function pack(){
mkdir -p bin_package
cd bin_package
WITHAVX=$1
WITHMKL=$2
if [ $WITHAVX = "ON" -a $WITHMKL = "OFF" ]; then
    mkdir -p serving-cpu-avx-openblas-$version
    cp ../build_server/core/general-server/serving  serving-cpu-avx-openblas-$version
    tar -czvf serving-cpu-avx-openblas-$version.tar.gz serving-cpu-avx-openblas-$version/
fi
if [ $WITHAVX = "OFF" -a $WITHMKL = "OFF" ]; then
    mkdir -p serving-cpu-noavx-openblas-$version
    cp ../build_server/core/general-server/serving serving-cpu-noavx-openblas-$version
    tar -czvf serving-cpu-noavx-openblas-$version.tar.gz serving-cpu-noavx-openblas-$version/
fi
if [ $WITHAVX = "ON" -a $WITHMKL = "ON" ]; then
    mkdir -p serving-cpu-avx-mkl-$version
    cp ../build_server/output/demo/serving/bin/serving serving-cpu-avx-mkl-$version
    cp ../build_server/third_party/install/Paddle/third_party/install/mklml/lib/* serving-cpu-avx-mkl-$version
    cp ../build_server/third_party/install/Paddle/third_party/install/mkldnn/lib/libdnnl.so.1 serving-gpu-$version 
    tar -czvf serving-cpu-avx-mkl-$version.tar.gz serving-cpu-avx-mkl-$version/
fi
cd ..
}

function pack_gpu(){
mkdir -p bin_package
cd bin_package
mkdir -p serving-gpu-$version
cp ../build_gpu_server/output/demo/serving/bin/serving serving-gpu-$version
cp ../build_gpu_server/third_party/install/Paddle/third_party/install/mklml/lib/* serving-gpu-$version
cp ../build_gpu_server/third_party/install/Paddle/third_party/install/mkldnn/lib/libdnnl.so.1 serving-gpu-$version
tar -czvf serving-gpu-$version.tar.gz serving-gpu-$version/
cd ..
}

function cp_whl(){
cp ./python/dist/paddle_serving_*-$version-py* ../whl_package
}

function compile_cpu(){
mkdir -p build_server
cd build_server
WITHAVX=$1
WITHMKL=$2
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ -DPYTHON_LIBRARY=$PYTHONROOT/lib64/libpython2.7.so -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python2.7 -DWITH_AVX=$WITHAVX -DWITH_MKL=$WITHMKL -DSERVER=ON .. > compile_log
make -j20 >> compile_log
make install >> compile_log
cp_whl
cd ..
pack $WITHAVX $WITHMKL
}

function compile_cpu_py3(){
mkdir -p build_server_py3
cd build_server_py3
WITHAVX=$1
WITHMKL=$2
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.6m/ -DPYTHON_LIBRARY=$PYTHONROOT/lib64/libpython3.6m.so -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.6m -DWITH_AVX=$WITHAVX -DWITH_MKL=$WITHMKL -DSERVER=ON .. > compile_log
make -j20 >> compile_log
#make install >> compile_log
cp_whl
cd ..
#pack $WITHAVX $WITHMKL
}

function compile_gpu(){
mkdir -p build_gpu_server
cd build_gpu_server
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ -DPYTHON_LIBRARY=$PYTHONROOT/lib64/libpython2.7.so -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python2.7 -DWITH_GPU=ON -DSERVER=ON -DCUDA_CUDART_LIBRARY=$CUDA_CUDART_LIBRARY .. > compile_log
make -j20 >> compile_log
make install >> compile_log
cp_whl
cd ..
pack_gpu
}


function compile_gpu_py3(){
mkdir -p build_gpu_server_py3
cd build_gpu_server_py3
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.6m/ -DPYTHON_LIBRARY=$PYTHONROOT/lib64/libpython3.6m.so -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.6m -DWITH_GPU=ON -DSERVER=ON .. > compile_log
make -j20 >> compile_log
#make install >> compile_log
cp_whl
cd ..
#pack_gpu
}

function compile_client(){
mkdir -p build_client
cd build_client
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ -DPYTHON_LIBRARY=$PYTHONROOT/lib64/libpython2.7.so -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python2.7 -DCLIENT=ON -DPACK=ON .. > compile_log
make -j20 >> compile_log
#make install >> compile_log
cp_whl
cd ..
}


function compile_client_py3(){
mkdir -p build_client_py3
cd build_client_py3
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.6m/ -DPYTHON_LIBRARY=$PYTHONROOT/lib64/libpython3.6m.so -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.6m -DCLIENT=ON -DPACK=ON .. > compile_log
make -j20 >> compile_log
#make install >> compile_log
cp_whl
cd ..
}

function compile_app(){
mkdir -p build_app
cd build_app
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ -DPYTHON_LIBRARIES=$PYTHONROOT/lib64/libpython2.7.so -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python -DAPP=ON .. > compile_log
make -j20 >> compile_log
#make install >> compile_log
cp_whl
cd ..
}

function compile_app_py3(){
mkdir -p build_app_py3
cd build_app_py3
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.6m/ -DPYTHON_LIBRARY=$PYTHONROOT/lib64/libpython3.6m.so -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.6m -DAPP=ON ..> compile_log
make -j20 >> compile_log
#make install >> compile_log
cp_whl
cd ..
}

function upload_bin(){
    cd bin_package
    python ../bos_conf/upload.py serving-cpu-avx-openblas-$version.tar.gz
    python ../bos_conf/upload.py serving-cpu-avx-mkl-$version.tar.gz
    python ../bos_conf/upload.py serving-cpu-noavx-openblas-$version.tar.gz
    python ../bos_conf/upload.py serving-gpu-$version.tar.gz
    cd ..
}

function upload_whl(){
    cd whl_package
    #python ../bos_conf/upload_whl.py paddle_serving_client-$version-py2-none-any.whl 
    #python ../bos_conf/upload_whl.py paddle_serving_server-$version-py2-none-any.whl  
    #python ../bos_conf/upload_whl.py paddle_serving_server_gpu-$version-py2-none-any.whl
    #python ../bos_conf/upload_whl.py paddle_serving_client-$version-py3-none-any.whl 
    #python ../bos_conf/upload_whl.py paddle_serving_server-$version-py3-none-any.whl  
    #python ../bos_conf/upload_whl.py paddle_serving_server_gpu-$version-py3-none-any.whl
    cd .. 
}

#cpu-avx-openblas $1-avx  $2-mkl
#compile_cpu ON OFF
#compile_cpu_py3 ON OFF

#cpu-avx-mkl
#compile_cpu ON ON
#compile_cpu_py3 ON ON

#cpu-noavx-openblas
#compile_cpu OFF OFF
#compile_cpu_py3 OFF OFF

#gpu
#compile_gpu
#compile_gpu_py3

#client
#compile_client
#compile_client_py3

#app
#compile_app
#compile_app_py3

#upload bin
#upload_bin

#upload whl
#upload_whl
