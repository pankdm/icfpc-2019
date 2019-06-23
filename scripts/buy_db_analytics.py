#!/usr/bin/env python3.7

import os
import re

from parser import read_yaml

FULL_ANALYTICS = True

GOLD = "../solutions_gold/"

def calculate_roi(n, base_yaml, compare_yaml):
    old_score = base_yaml["time"]
    new_score = compare_yaml["time"]
    max_score = compare_yaml["max_score"]
    delta = (max_score * (old_score - new_score) / old_score)

    buy = compare_yaml["buy"]
    spent = compare_yaml["spent"]
    roi = delta / spent
    msg = (f"Task {n}: {buy} ({spent}): " +
           f"{old_score} -> {new_score} = (+{delta:.1f}), max {max_score}, " +
           f"ROI = {roi * 100:.1f}%")
    # print (msg)
    return roi, msg


def get_best_roi_clone(n):
    # print ()
    # print (f"Task {n}")
    base = f"{GOLD}/prob-{n:03}.meta.yaml"
    base_yaml = read_yaml(base)

    rois = []

    folder = f"../buy_db/task{n}/"
    files = os.listdir(folder)
    for f in sorted(files):
        s = re.match("(\d+).meta.yaml", f)
        if s:
            compare_yaml = read_yaml(folder + f)
            roi, msg = calculate_roi(n, base_yaml, compare_yaml)
            if compare_yaml["buy"] != "C":
                continue
            rois.append((roi, s.group(1)))

    rois.sort(key = lambda x : x[0], reverse=True)
    return rois[0]

def process_task(n):
    # print ()
    # print (f"Task {n}")
    base = f"{GOLD}/prob-{n:03}.meta.yaml"
    base_yaml = read_yaml(base)

    rois = []

    folder = f"../buy_db/task{n}/"
    files = os.listdir(folder)
    for f in sorted(files):
        s = re.match("(\d+).meta.yaml", f)
        if s:
            compare_yaml = read_yaml(folder + f)
            roi, msg = calculate_roi(n, base_yaml, compare_yaml)
            rois.append((roi, msg))

    rois.sort(key = lambda x : x[0], reverse=True)
    if FULL_ANALYTICS:
        print()
        for roi in rois:
            print (roi[1])
    else:
        print (f"{rois[0][1]}")

BALANCE = 339000

def select_manual():
    os.system("mkdir -p ../solutions/manual/")
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
        remaining -= 2000
        os.system(f"cp ../buy_db/task{i}/{index}.buy ../solutions_manual/prob-{i:03}.buy")
        os.system(f"cp ../buy_db/task{i}/{index}.sol ../solutions_manual/prob-{i:03}.sol")

if __name__ == "__main__":
    # select_manual()
    for i in range(1, 301):
        process_task(i)
