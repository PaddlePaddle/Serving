## Timeline Tool Tutorial

([简体中文](./README_CN.md)|English)

The serving framework has a built-in function for predicting the timing of each stage of the service. The client controls whether to turn on the environment through environment variables. After opening, the information will be output to the screen.
```
export FLAGS_profile_client=1 #turn on the client timing tool for each stage
export FLAGS_profile_server=1 #turn on the server timing tool for each stage
```
After enabling this function, the client will print the corresponding log information to standard output during the prediction process.

In order to show the time consuming of each stage more intuitively, a script is provided to further analyze and process the log file.

When using, first save the output of the client to a file, taking `profile` as an example.
```
python3 show_profile.py profile ${thread_num}
```
Here the `thread_num` parameter is the number of processes when the client is running, and the script will calculate the average time spent in each phase according to this parameter.

The script calculates the time spent in each stage, divides by the number of threads to average, and prints to standard output.

```
python3 timeline_trace.py profile trace
```
The script converts the time-dot information in the log into a json format and saves it to a trace file. The trace file can be visualized through the tracing function of the Chrome browser.

Specific operation: Open the chrome browser, enter `chrome://tracing/` in the address bar, jump to the tracing page, click the `load` button, and open the saved trace file to visualize the time information of each stage of the prediction service.

The data visualization output is shown as follow, it uses [bert as service example](../C++/PaddleNLP/bert) GPU inference service. The server starts 4 GPU prediction, the client starts 4 `processes`, and the timeline of each stage when the batch size is 1. Among them, `bert_pre` represents the data preprocessing stage of the client, and `client_infer` represents the stage where the client completes sending and receiving prediction requests. `process` represents the process number of the client, and the second line of each process shows the timeline of each op of the server.

![timeline](../../doc/images/timeline-example.png)
