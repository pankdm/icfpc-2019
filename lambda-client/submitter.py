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

    subprocess.check_call(
         [python, "../scripts/puzzle_solver.py", puzzleIn, puzzleOut])

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
    for block in range(1, 16):
        task = subprocess.check_output(
            [python, cli, "getblockinfo", str(block)]).decode()
        solve(task)

while True:
    task = subprocess.check_output(
        [python, cli, "getblockinfo"]).decode()
    solve(task)
