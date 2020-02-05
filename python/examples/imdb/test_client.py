from paddle_serving_client import Client
import sys

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9292"])

for line in sys.stdin:
    group = line.strip().split()
    words = [int(x) for x in group[1:int(group[0]) + 1]]
    label = [int(group[-1])]
    feed = {"words": words, "label": label}
    fetch = ["cost", "acc", "prediction"]
    fetch_map = client.predict(feed=feed, fetch=fetch)
    print(fetch_map)
    #print("{} {}".format(fetch_map["prediction"][1], label[0]))
    
