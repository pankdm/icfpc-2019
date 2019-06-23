#!/usr/bin/env python3.7

import sys
import subprocess
from json import dumps, loads
import os
import time

from slack_integration import post_to_slack

python = "python3.7"

fnameState = "state.json"
cli = "./lambda-cli.py"

state = {}
state["submitted"] = []

if len(sys.argv) > 1:
    if sys.argv[1] == '0':
        SUBMIT = False
else:
    SUBMIT = True
print ('Submit = ', SUBMIT)

def loadState():
    if os.path.isfile(fnameState):
        with open(fnameState) as fState:
            global state
            state = loads(fState.read())

def print_and_log(logs, s):
    print (s)
    logs.append(s)

iteration = 0
def send_logs(logs):
    post_to_slack("```\n{}\n```".format("\n".join(logs)))


def output_exist(blockId):
    taskOut = "data/task%s.sol" % str(blockId)
    puzzleOut = "data/puzzle%s.desc" % str(blockId)
    return os.path.isfile(taskOut) and os.path.isfile(puzzleOut)

def solve(task, check_prev=False):
    logs = []
    logs.append(f"iteration = {iteration}")

    loadState()

    task = task.replace("\'", "\"")

    blockinfo = loads(task)

    if not "block" in blockinfo:
        print("Bad task", task)
        return

    blockId = blockinfo["block"]
    print_and_log(logs, f"Solving blockId {blockId}")
    if blockId in state["submitted"]:
        return

    if check_prev:
        prev_block_id = blockId - 1
        if not output_exist(prev_block_id):
            post_to_slack(f"<!here>, missing solution for previous block {prev_block_id}")

    fnameTask = "data/task%s.json" % str(blockId)
    with open(fnameTask, "w") as fTask:
        fTask.write(task)

    # print(task)
    print_and_log (logs, "puzzle task: {}".format(blockinfo["puzzle"].split("#")[0]))
    if "15" in blockinfo["balances"]:
        balance = blockinfo["balances"]["15"]
        print_and_log(logs, f"balance = {balance}")

    state["submitted"].append(blockId)

    if not output_exist(blockId):
        send_logs(logs)

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

    if SUBMIT:
        args = [python, cli, "submit",
                               str(blockId), taskOut, puzzleOut]
        print(args)
        subprocess.check_call(args)

        with open(fnameState, "w") as fState:
            fState.write(dumps(state))


if True:
    for block in range(25, 25):
        print("replay %d" % block)
        task = subprocess.check_output(
            [python, cli, "getblockinfo", str(block)]).decode()
        solve(task)

i = 0
while True:
    task = subprocess.check_output(
        [python, cli, "getblockinfo"]).decode()
    print("hb %d" % i)
    solve(task, check_prev=True)
    i += 1
    iteration += 1
    time.sleep(10)
