#!/usr/bin/env python3.7

from buys_solve_one import solve_one

import sys

# boosters = ["C", "B"]
# boosters = ["CC"]

boosters = ["CCC", "CCCC"]
RANGE = range(1, 221)

NUM_SHARDS = 5
shard = int(sys.argv[1])
assert 0 <= shard < NUM_SHARDS

for i in RANGE:
    if i % NUM_SHARDS == shard:
        for b in boosters:
            print ()
            print (f"Task {i}, buy {b}")
            solve_one(i, b)
