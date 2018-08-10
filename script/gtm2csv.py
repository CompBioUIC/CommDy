import fileinput
import re

print("time,group,individual")
for line in fileinput.input():
    fields = re.split("[ ,]", line.strip())
    g,t = fields[:2]
    members = fields[2:]
    for m in members:
        if m == "": continue
        print("%s,%s,%s"%(g, t, m))
