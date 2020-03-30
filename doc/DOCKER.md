# Docker compilation environment preparation

([简体中文](./DOCKER_CN.md)|English)

## Environmental requirements

+ Docker is installed on the development machine.
+ Compiling the GPU version requires nvidia-docker.

## Dockerfile

[CPU Version Dockerfile](../tools/Dockerfile)

[GPU Version Dockerfile](../tools/Dockerfile.gpu)

## Instructions

### Building Docker Image

Create a new directory and copy the Dockerfile to this directory.

Run

```bash
docker build -t serving_compile:cpu .
```

Or

```bash
docker build -t serving_compile:cuda9 .
```

## Enter Docker Container

CPU Version please run

```bash
docker run -it serving_compile:cpu bash
```

GPU Version please run

```bash
docker run -it --runtime=nvidia -it serving_compile:cuda9 bash
```

##  List of supported environments compiled by Docker

The list of supported environments is as follows:：

| System Environment Supported by CPU Docker Compiled Executables |
| -------------------------- |
| Centos6                    |
| Centos7                    |
| Ubuntu16.04                |
| Ubuntu18.04               |



| System Environment Supported by GPU Docker Compiled Executables |
| ---------------------------------- |
| Centos6_cuda9_cudnn7                       |
| Centos7_cuda9_cudnn7                  |
| Ubuntu16.04_cuda9_cudnn7                       |
| Ubuntu16.04_cuda10_cudnn7                  |



**Remarks:**
+ If you cannot find libcrypto.so.10 and libssl.so.10 when you execute the pre-compiled version, you can change /usr/lib64/libssl.so.10 and /usr/lib64/libcrypto.so in the Docker environment. 10 Copy to the directory where the executable is located.
+ CPU pre-compiled version can only be executed on CPU machines, GPU pre-compiled version can only be executed on GPU machines.
