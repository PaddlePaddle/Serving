# C++ Serving 异步框架

Paddle Serving 的网络框架层面是同步处理模式，即 bRPC 网络处理线程从系统内核拿到完整请求数据后( epoll 模式)，在同一线程内完成业务处理，C++ Serving 默认使用同步模式。同步模式比较简单直接，适用于模型预测时间短，或单个 Request 请求批量较大的情况。

<p align="center">
<img src='../images/syn_mode.png' width = "350" height = "300">
<p>

Server 端线程数 N = 模型预测引擎数 N = 同时处理 Request 请求数 N，超发的 Request 请求需要等待当前线程处理结束后才能得到响应和处理。


为了提高计算芯片吞吐和计算资源利用率，C++ Serving 在调度层实现异步多线程并发合并请求，实现动态批量推理。异步模型主要适用于模型支持批量，单个 Request 请求的无批量或较小，单次预测时间较长的情况。

<p align="center">
<img src='../images/asyn_mode.png'>
<p>

异步模式下，Server 端 N 个线程只负责接收 Request 请求，实际调用预测引擎是在异步框架的线程池中，异步框架的线程数可以由配置选项来指定。为了方便理解，我们假设每个 Request 请求批量均为1，此时异步框架会尽可能多得从请求池中取 n(n≤M)个 Request 并将其拼装为1个 Request(batch=n)，调用1次预测引擎，得到1个 Response(batch = n)，再将其对应拆分为 n 个 Response 作为返回结果。
