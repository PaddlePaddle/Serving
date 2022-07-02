RUN_ENV=$1
if [[ "$RUN_ENV" == "cuda10.1" ]];then
    ln -sf /usr/local/cuda-10.1/targets/x86_64-linux/lib/libcudart.so.10.1 /usr/lib/libcudart.so && \
    ln -sf /usr/local/cuda-10.1/targets/x86_64-linux/lib/libcusolver.so.10 /usr/lib/libcusolver.so && \
    ln -sf /usr/lib/x86_64-linux-gnu/libcuda.so /usr/lib/libcuda.so && \
    ln -sf /usr/lib/x86_64-linux-gnu/libcublas.so.10 /usr/lib/libcublas.so && \
    ln -sf /usr/lib/x86_64-linux-gnu/libcudnn.so.7 /usr/lib/libcudnn.so

elif [[ "$RUN_ENV" == "cuda10.2" ]];then
    ln -sf /usr/local/cuda-10.2/targets/x86_64-linux/lib/libcudart.so.10.2 /usr/lib/libcudart.so && \
    ln -sf /usr/local/cuda-10.2/targets/x86_64-linux/lib/libcusolver.so.10 /usr/lib/libcusolver.so && \
    ln -sf /usr/lib/x86_64-linux-gnu/libcuda.so /usr/lib/libcuda.so && \
    ln -sf /usr/lib/x86_64-linux-gnu/libcublas.so.10 /usr/lib/libcublas.so && \
    ln -sf /usr/lib/x86_64-linux-gnu/libcudnn.so.8 /usr/lib/libcudnn.so

elif [[ "$RUN_ENV" == "cuda11" ]];then
    ln -sf /usr/local/cuda-11.0/targets/x86_64-linux/lib/libcudart.so.11.0 /usr/lib/libcudart.so && \
    ln -sf /usr/local/cuda-11.0/targets/x86_64-linux/lib/libcusolver.so.10 /usr/lib/libcusolver.so && \
    ln -sf /usr/lib/x86_64-linux-gnu/libcuda.so /usr/lib/libcuda.so && \
    ln -sf /usr/local/cuda-11.0/targets/x86_64-linux/lib/libcublas.so.11 /usr/lib/libcublas.so && \
    ln -sf /usr/lib/x86_64-linux-gnu/libcudnn.so.8 /usr/lib/libcudnn.so
fi
