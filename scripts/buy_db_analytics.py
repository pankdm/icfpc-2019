#!/usr/bin/env python3.7

import os
import re

from parser import read_yaml

FULL_ANALYTICS = True

GOLD = "../solutions_gold/"

def calculate_roi(n, index, base_yaml, compare_yaml):
    old_score = base_yaml["time"]
    new_score = compare_yaml["time"]
    max_score = compare_yaml["max_score"]
    delta = (max_score * (old_score - new_score) / old_score)

    buy = compare_yaml["buy"]
    spent = compare_yaml["spent"]
    if spent != 0:
        roi = (delta - spent) / spent
    else:
        roi = 0
    sign = "+" if delta > 0 else ""
    msg = (f"Task {n},{index}: {buy} ({spent}) " +
           f"{old_score} -> {new_score} = ({sign}{delta:.1f}), max {max_score}, " +
           f"ROI = {roi * 100:.1f}%")
    # print (msg)
    return roi, msg


def get_best_roi_clone(n, buy):
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
            if not compare_yaml:
                continue

            roi, msg = calculate_roi(n, s.group(1), base_yaml, compare_yaml)
            if compare_yaml["buy"] != buy:
                continue
            rois.append((roi, s.group(1)))

    rois.sort(key = lambda x : x[0], reverse=True)
    if rois:
        return rois[0]
    else:
        return [], None

def process_task(n):
    # print ()
    # print (f"Task {n}")
    base = f"{GOLD}/prob-{n:03}.meta.yaml"
    base_yaml = read_yaml(base)

    rois = {}

    folder = f"../buy_db/task{n}/"
    files = os.listdir(folder)
    for f in sorted(files):
        s = re.match("(\d+).meta.yaml", f)
        if s:
            compare_yaml = read_yaml(folder + f)
            if not compare_yaml:
                continue

            roi, msg = calculate_roi(n, s.group(1), base_yaml, compare_yaml)
            buy = compare_yaml["buy"]
            if buy in rois:
                prev = rois[buy]
                if roi > prev[0]:
                    rois[buy] = (roi, msg)
            else:
                rois[buy] = (roi, msg)


    rois_values = list(rois.values())
    rois_values.sort(key = lambda x : x[0], reverse=True)
    if FULL_ANALYTICS:
        print()
        for roi in rois_values:
            print (roi[1])
    else:
        print (f"{rois[0][1]}")

if __name__ == "__main__":
    # select_manual()
    for i in range(1, 301):
        process_task(i)
