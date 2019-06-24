#!/usr/bin/env python3.7

from buys_solve_one import solve_one

import sys
import time

# boosters = ["C", "B"]
# boosters = ["CC"]


FULL_RANGE = range(1, 301)
NON_CLONE_RANGE = range(1, 220)

class BuyTask:
    def __init__(self, range, boosters):
        self.range = range
        self.boosters = boosters


tasks = [
    # BuyTask(FULL_RANGE, ["BC"]),
    BuyTask(FULL_RANGE, [""]),
    BuyTask(FULL_RANGE, ["B"]),
    BuyTask(NON_CLONE_RANGE, ["C"]),
    BuyTask(FULL_RANGE, ["BB"]),
    # BuyTask(FULL_RANGE, ["CC"]),
    # BuyTask(NON_CLONE_RANGE, ("CCC", "CCCC")),
]


shard = int(sys.argv[1])
NUM_SHARDS = int(sys.argv[2])

assert 0 <= shard < NUM_SHARDS

log_file = open(f"log.buy.{shard}", "w")
log_file.close()

prog_start = time.time()

for task in tasks:
    for i in task.range:
        if i % NUM_SHARDS == shard:
            for b in task.boosters:
                with open(f"log.buy.{shard}", "a") as log_file:
                    start = time.time()
                    delta0 = int(start - prog_start)
                    log_file.write(f"{delta0}s: Task {i}, buy {b}\n")
                    solve_one(i, b)
                    delta = time.time() - start
                    log_file.write(f"   finished in {delta:.3f}s\n")
