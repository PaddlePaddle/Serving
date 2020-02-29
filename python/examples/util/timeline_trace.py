#coding=utf-8
import json
import sys

profile_file = sys.argv[1]


def prase(line, counter):
    event_list = line.split(" ")
    trace_list = []
    for event in event_list:
        name, ts = event.split(":")
        name_list = name.split("_")
        ph = "B" if (name_list[-1] == "0") else "E"
        if len(name_list) == 2:
            name = name_list[0]
        else:
            name = name_list[0] + "_" + name_list[1]
        event_dict = {}
        event_dict["name"] = name
        event_dict["tid"] = 0
        event_dict["pid"] = 0
        event_dict["ts"] = ts
        event_dict["ph"] = ph

        trace_list.append(event_dict)
    return trace_list


if __name__ == "__main__":
    profile_file = sys.argv[1]
    trace_file = sys.argv[2]
    all_list = []
    counter = 0
    with open(profile_file) as f:
        for line in f.readlines():
            line = line.strip().split("\t")
            if line[0] == "PROFILE":
                trace_list = prase(line[1], counter)
                counter += 1
                for trace in trace_list:
                    all_list.append(trace)

    trace = json.dumps(all_list, indent=2, separators=(',', ':'))
    with open(trace_file, "w") as f:
        f.write(trace)
