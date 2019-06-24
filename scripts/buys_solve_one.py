#!/usr/bin/env python3.7

import os
import re
import subprocess
import math
from yaml import safe_load, dump
import time

from api import read_file
from parser import parse_problem, parse_solution


STORAGE = "../buy_db"
PROBLEMS = "../problems/all"
TAG = "fair_mops"

def get_new_id(folder):
    files = os.listdir(folder)
    if not files:
        return 1

    max_id = 1
    for f in files:
        s = re.match("(\d+).buy", f)
        if s:
            max_id = max(max_id, int(s.group(1)))
    return max_id + 1

COST = {
    "B": 1000,
    "F": 300,
    "L": 700,
    "R": 1200,
    "C": 2000,
}


def money_spent(boosters):
    spent = 0
    for b in boosters:
        spent += COST[b]
    return spent

def get_max_score(world):
    box = world.mappa.get_box()
    return int(round(1000 * math.log(box[0][1] * box[1][1], 2)))


# n - task number
# boosters - string of extra boosters
def solve_one(n, boosters):
    print ("Solving ", n, " boosters")
    folder = f"{STORAGE}/task{n}/"
    os.system(f"mkdir -p {folder}")
    new_id = get_new_id(folder)

    buy_in = f"{folder}/{new_id}.buy"
    task_in = f"{PROBLEMS}/prob-{n:03}.desc"
    task_out = f"{folder}/{new_id}.sol"
    meta_out = f"{folder}/{new_id}.meta.yaml"

    with open(buy_in, "w") as buy_file:
        buy_file.write(boosters)

    extra_args = []
    if os.path.isfile(task_out):
        extra_args = ["-current_best", task_out]

    subprocess.check_call(
        ["../src/build/cpp_solver",
            "-solve", "1",
            "-in", task_in,
            "-out", task_out +
            (["-bonus", buy_in] if boosters else []) +
            extra_args)

    task_in_content = read_file(task_in)
    world = parse_problem(task_in_content)

    task_out_content = read_file(task_out)
    sol_time = len(parse_solution(task_out_content))

    with open(meta_out, "w") as meta_file:
        metadata = {
            "time": sol_time,
            "task_id": n,
            "buy": boosters,
            "spent": money_spent(boosters),
            "max_score": get_max_score(world),
            "ts": int(time.time()),
            "tag": TAG,
        }
        dump(metadata, meta_file)


if __name__ == "__main__":
    solve_one(2, "C")
