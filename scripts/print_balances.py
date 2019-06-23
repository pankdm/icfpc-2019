#!/usr/bin/env python3

import json
import os

for i in range(1, 200):
    filename = "../lambda-client/blocks/%d/balances.json" % i
    if not os.path.isfile(filename):
        continue
    s = open(filename).read()
    balance = 0
    j = json.loads(s)

pairs = list(j.items())
pairs = sorted(pairs, key=lambda x: -x[1])
for k, v in pairs:
    print("%s\t%s" % (str(k), str(v)))
