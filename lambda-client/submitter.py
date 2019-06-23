#!/usr/bin/env python3.7

import sys
import subprocess
from json import dumps, loads

python = "python3.7"

while True:
    task = subprocess.check_output([python, "./lambda-cli.py", "getblockinfo"]).decode()
    # for s in ['balances', 'block', "block_subs", "puzzle", "block_ts", "excluded"]:
    #     task = task.replace("\'" + s + "\'", "\"" + s + "\"")
    task = task.replace("\'", "\"")
    print(task)
    blockinfo = loads(task)

    blockId = blockinfo["block"]

    puzzleIn = "%s_task.desc" % blockId
    puzzleOut = "%s_task.desc" % blockId
    with open() as fTask:
        fTask.write(blockinfo["puzzle"])

    subprocess.check_call([python, "../scripts/puzzle_solver.py", puzzleIn, puzzleOut])

    subprocess.check_call([python, "./lambda-cli.py", "-block", blockId, "-puzzle_sol_path", puzzleOut])
