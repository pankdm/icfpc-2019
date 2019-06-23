#!/usr/bin/env python3.7

import sys
import subprocess
from json import dumps, loads
import os

python = "python3.7"

fnameState = "state.json"

state = {}
state["submitted"] = []

def solve(task):
    task = task.replace("\'", "\"")
    print(task)
    blockinfo = loads(task)

    blockId = blockinfo["block"]

    if blockId in state["submitted"]:
        return

    state["submitted"].append(blockId)

    puzzleIn = "data/puzzle%s.task" % str(blockId)
    puzzleOut = "data/puzzle%s.desc" % str(blockId)
    with open(puzzleIn, "w") as fTask:
        fTask.write(blockinfo["puzzle"])

    # subprocess.check_call(
    #     [python, "../scripts/puzzle_solver.py", puzzleIn, puzzleOut])

    with open(puzzleOut, "w") as fTemp:
        pass

    taskIn = "data/task%s.desc" % str(blockId)
    taskOut = "data/task%s.sol" % str(blockId)
    with open(taskIn, "w") as fTask:
        fTask.write(blockinfo["task"])

    subprocess.check_call(
        ["../src/build/cpp_solver", "-solve", "1", "-in", taskIn, "-out", taskOut])

    args = [python, "./lambda-cli.py", "submit",
                           str(blockId), taskOut, puzzleOut]
    print(args)
    subprocess.check_call(args)

    with open(fnameState, "w") as fState:
        fState.write(dumps(state))

def loadState():
    if os.path.isfile(fnameState):
        with open(fnameState) as fState:
            state = loads(fState.read())

if False:
    for block in range(12, 14):
        loadState()

        task = subprocess.check_output(
            [python, "./lambda-cli.py", "getblockinfo", str(block)]).decode()
        solve(task)

while True:
    loadState()

    task = subprocess.check_output(
        [python, "./lambda-cli.py", "getblockinfo"]).decode()
    solve(task)
