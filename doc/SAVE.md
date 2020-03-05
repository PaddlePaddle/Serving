## How to save a servable model of Paddle Serving?
- Currently, paddle serving provides a save_model interface for users to access, the interface is similar with `save_inference_model` of Paddle.
``` python
import paddle_serving_client.io as serving_io
serving_io.save_model("imdb_model", "imdb_client_conf",
                      {"words": data}, {"prediction": prediction},
                      fluid.default_main_program())
```
`imdb_model` is the server side model with serving configurations. `imdb_client_conf` is the client rpc configurations. Serving has a 
dictionary for `Feed` and `Fetch` variables for client to assign. An alias name can be defined for each variable. An example of how to use alias name
 is as follows:
 ``` python
 from paddle_serving_client import Client
import sys

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9393"])

for line in sys.stdin:
    group = line.strip().split()
    words = [int(x) for x in group[1:int(group[0]) + 1]]
    label = [int(group[-1])]
    feed = {"words": words, "label": label}
    fetch = ["acc", "cost", "prediction"]
    fetch_map = client.predict(feed=feed, fetch=fetch)
    print("{} {}".format(fetch_map["prediction"][1], label[0]))
 ```
