#coding=utf-8
import sys
import collections

profile_file = sys.argv[1]
thread_num = sys.argv[2]
time_dict = collections.OrderedDict()
query_count = 0


def prase(line):
    profile_list = line.split(" ")
    num = len(profile_list)
    for idx in range(int(num / 2)):
        profile_0_list = profile_list[idx * 2].split(":")
        profile_1_list = profile_list[idx * 2 + 1].split(":")
        if len(profile_0_list[0].split("_")) == 2:
            name = profile_0_list[0].split("_")[0]
        else:
            name = profile_0_list[0].split("_")[0] + "_" + profile_0_list[
                0].split("_")[1]
        cost = int(profile_1_list[1]) - int(profile_0_list[1])
        if name not in time_dict:
            time_dict[name] = cost
        else:
            time_dict[name] += cost


with open(profile_file) as f:
    query_count = 0
    for line in f.readlines():
        line = line.strip().split("\t")
        if line[0] == "PROFILE":
            prase(line[2])
            query_count += 1

print("thread_num: {}".format(thread_num))
print("query_count: {}".format(query_count))
for name in time_dict:
    print("{} cost: {}s in each thread ".format(name, time_dict[name] / (
        1000000.0 * float(thread_num))))
