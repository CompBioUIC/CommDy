import fileinput

print("time,group,individual")
for line in fileinput.input():
    fields = line.strip().split(" ")
    g,t = fields[:2]
    members = fields[2:]
    for m in members:
        print("%s,%s,%s"%(g, t, m))
