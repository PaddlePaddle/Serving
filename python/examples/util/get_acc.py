import sys
import os

total = 0
acc = 0
for line in sys.stdin:
    line = line.strip()
    group = line.split()
    if (float(group[0]) - 0.5) * (float(group[1]) - 0.5) > 0:
        acc += 1
    total += 1

print(float(acc) / float(total))
