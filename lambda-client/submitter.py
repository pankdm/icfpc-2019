#!/usr/bin/env python3.7

import sys
import subprocess
from json import dumps, loads
import os

python = "python3.7"

fnameState = "state.json"

state = {}
state["submitted"] = []

while True:
    if os.path.isfile(fnameState):
        with open(fnameState) as fState:
            state = loads(fState.read())

    task = subprocess.check_output(
        [python, "./lambda-cli.py", "getblockinfo"]).decode()
    # for s in ['balances', 'block', "block_subs", "puzzle", "block_ts", "excluded"]:
    #     task = task.replace("\'" + s + "\'", "\"" + s + "\"")
    task = task.replace("\'", "\"")
    print(task)
    blockinfo = loads(task)

    blockId = blockinfo["block"]

    if blockId in state["submitted"]:
        continue

    state["submitted"].append(blockId)

    puzzleIn = "data/%s_puzzle.desc" % blockId
    puzzleOut = "data/%s_puzzle.res" % blockId
    with open(puzzleIn, "w") as fTask:
        fTask.write(blockinfo["puzzle"])

    # subprocess.check_call(
    #     [python, "../scripts/puzzle_solver.py", puzzleIn, puzzleOut])

    with open(puzzleOut, "w") as fTemp:
        pass

    taskIn = "data/%s_task.desc" % blockId
    taskOut = "data/%s_task.res" % blockId
    with open(taskIn, "w") as fTask:
        fTask.write(blockinfo["task"])

    subprocess.check_call(
        ["../src/build/cpp_solver", "-solve", "1", "-in", taskIn, "-out", taskOut])

    subprocess.check_call([python, "./lambda-cli.py", "submit", "-block",
                           blockId, "-puzzle_sol_path", puzzleOut, "-task_sol_path", taskOut])

    with open(fnameState, "w") as fState:
        fState.write(dumps(state))
