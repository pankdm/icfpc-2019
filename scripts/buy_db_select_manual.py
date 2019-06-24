#!/usr/bin/env python3.7

import os
import re


from buy_db_analytics import get_best_roi_clone

BALANCE = 339000

def select_manual():
    os.system("mkdir -p ../solutions_manual/")
    selected = []
    for i in range(1, 301):
        # process_task(i)
        roi, index = get_best_roi_clone(i)
        selected.append((roi, i, index))
    selected.sort(key = lambda x : x[0], reverse=True)

    for i in range(1, 301):
        os.system(f"cp ../solutions_gold/prob-{i:03}.sol ../solutions_manual/")

    remaining = BALANCE
    for t in selected:
        if remaining < 0:
            break
        roi, i, index = t
        print(f"{100 * roi:.1f}%, task{i}")
        remaining -= 2000
        os.system(f"cp ../buy_db/task{i}/{index}.buy ../solutions_manual/prob-{i:03}.buy")
        os.system(f"cp ../buy_db/task{i}/{index}.sol ../solutions_manual/prob-{i:03}.sol")


if __name__ == "__main__":
    select_manual();
