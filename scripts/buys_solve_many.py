#!/usr/bin/env python3.7

from buys_solve_one import solve_one


boost = ["C", "B"]

# boosters = "CC"
# boosters = "CB"
# boosters = "BB"

for i in range(1, 301):
    for b in boost:
        solve_one(i, b)
