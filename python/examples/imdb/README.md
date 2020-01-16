### 使用方法

假设数据文件为test.data，配置文件为inference.conf

单进程client
```
cat test.data | python test_client.py > result
```
多进程client，若进程数为4
```
python test_client_multithread.py inference.conf test.data 4 > result
```
batch clienit
