#!/usr/bin/env python3.7

import os
import re

from parser import read_yaml

FULL_ANALYTICS = True

GOLD = "../solutions_gold/"

def print_score(n, base_yaml, compare_yaml):
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
            rois.append(print_score(n, base_yaml, compare_yaml))

    rois.sort(key = lambda x : x[0], reverse=True)
    if FULL_ANALYTICS:
        print()
        for roi in rois:
            print (roi[1])
    else:
        print (f"{rois[0][1]}")

if __name__ == "__main__":
    for i in range(1, 300):
        process_task(i)
