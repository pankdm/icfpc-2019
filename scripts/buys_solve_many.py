#!/usr/bin/env python3.7

from buys_solve_one import solve_one

import sys

# boosters = ["C", "B"]
# boosters = ["CC"]



FULL_RANGE = range(1, 301)
NON_CLONE_RANGE = range(1, 220)

class BuyTask:
    def __init__(self, range, boosters):
        self.range = range
        self.boosters = boosters


tasks = [
    BuyTask(FULL_RANGE, ("C", "CC")),
    BuyTask(NON_CLONE_RANGE, ("CCC", "CCCC")),
]


NUM_SHARDS = 5
shard = int(sys.argv[1])
assert 0 <= shard < NUM_SHARDS


for task in tasks:
    for i in task.range:
        if i % NUM_SHARDS == shard:
            for b in task.boosters:
                print ()
                print (f"Task {i}, buy {b}")
                solve_one(i, b)
