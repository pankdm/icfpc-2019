#!/usr/bin/env python3.7

from buys_solve_one import solve_one

import sys

# boosters = ["C", "B"]
boosters = ["CC"]

NUM_SHARDS = 5
shard = int(sys.argv[1])
assert 0 <= shard < NUM_SHARDS

for i in range(1, 301):
    if i % NUM_SHARDS == shard:
        for b in boost:
            print ()
            print ('Task {i}, buy {b}')
            solve_one(i, b)
