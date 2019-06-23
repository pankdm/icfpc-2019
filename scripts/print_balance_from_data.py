#!/usr/bin/env python3

import json
import os

print ("block\tbalance\tdelta\texcluded")

prev_balance = 0
for i in range(1, 100):
    filename = "../lambda-client/data/task%d.json" % i
    if not os.path.isfile(filename):
        continue
    s = open(filename).read()
    balance = 0
    j = json.loads(s)
    jb = j.get("balances", {})
    if "15" in jb:
        balance = jb["15"]

    excluded = j.get("excluded", [-1])[0]
    delta = balance - prev_balance
    prev_balance = balance
    print (f"{i}\t{balance}\t{delta}\t{excluded}")
