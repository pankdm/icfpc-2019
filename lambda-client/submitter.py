#!/usr/bin/env python3.7

import sys
import subprocess
from json import dumps, loads
import os

python = "python3.7"

fnameState = "state.json"
cli = "./lambda-cli.py"

state = {}
state["submitted"] = []

def loadState():
    if os.path.isfile(fnameState):
        with open(fnameState) as fState:
            global state
            state = loads(fState.read())

def solve(task):
    loadState()

    task = task.replace("\'", "\"")

    blockinfo = loads(task)

    if not "block" in blockinfo:
        print("Bad task", task)
        return

    blockId = blockinfo["block"]

    if blockId in state["submitted"]:
        return

    print(task)
    if "15" in blockinfo["balances"]:
        print('balance', blockinfo["balances"]["15"])

    state["submitted"].append(blockId)

    puzzleIn = "data/puzzle%s.task" % str(blockId)
    puzzleOut = "data/puzzle%s.desc" % str(blockId)
    with open(puzzleIn, "w") as fTask:
        fTask.write(blockinfo["puzzle"])

    res = 1
    it = 0
    while res != 0 and it < 50:
        print("puzzle_solver %d" % it)
        res = subprocess.call(
            [python, "../scripts/puzzle_solver.py", puzzleIn, puzzleOut])
        it += 1

    taskIn = "data/task%s.desc" % str(blockId)
    taskOut = "data/task%s.sol" % str(blockId)
    with open(taskIn, "w") as fTask:
        fTask.write(blockinfo["task"])

    subprocess.check_call(
        ["../src/build/cpp_solver", "-solve", "1", "-in", taskIn, "-out", taskOut])

    args = [python, cli, "submit",
                           str(blockId), taskOut, puzzleOut]
    print(args)
    subprocess.check_call(args)

    with open(fnameState, "w") as fState:
        fState.write(dumps(state))

if True:
    for block in range(18, 25):
        task = subprocess.check_output(
            [python, cli, "getblockinfo", str(block)]).decode()
        solve(task)

i = 0
while True:
    task = subprocess.check_output(
        [python, cli, "getblockinfo"]).decode()
    print("hb %d" % i)
    solve(task)
    i += 1
