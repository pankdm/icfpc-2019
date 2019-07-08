import os
import re

import json

from parser import read_yaml, read_file


ALLOWED_BUYS = ["", "C", "CC", "CCC", "CCCC"]
BEST_TIME_BUYS = ["", "C", "CC"]
# BEST_TIME_BUYS = ["", "C"]


def update_roi(gold_time, max_score, best_time, item):
    cur_time = item["time"]
    spent = item["spent"]
    prev_score = max_score * (best_time / gold_time)
    now_score = max_score * (best_time / cur_time)
    delta = now_score - prev_score

    if spent != 0:
        roi = (delta - spent) / spent
    else:
        roi = 0

    item["delta"] = delta
    item["roi"] = roi


# returns map of costs for a task
def process_task(n):
    base = f"../solutions_gold/prob-{n:03}.meta.yaml"
    base_yaml = read_yaml(base)

    best_time = base_yaml["time"]


    result = {}
    result["gold"] = base_yaml["time"]
    result["task"] = n

    # read buy db and find the best result for each buy
    best = {}
    folder = f"../buy_db/task{n}/"
    files = os.listdir(folder)
    max_score = None
    for f in sorted(files):
        s = re.match("(\d+).meta.yaml", f)
        if s:
            current_yaml = read_yaml(folder + f)
            if not current_yaml:
                continue

            buy = current_yaml["buy"]
            if buy not in ALLOWED_BUYS:
                continue

            if max_score is None:
                max_score = current_yaml["max_score"]

            if buy in BEST_TIME_BUYS:
                best_time = min(best_time, current_yaml["time"])

            take = False
            if buy not in best:
                take = True
            else:
                prev = best[buy]
                if prev["time"] > current_yaml["time"]:
                    take = True

            if take:
                item = {}
                item["time"] = current_yaml["time"]
                item["spent"] = current_yaml["spent"]
                best[buy] = item

    result["buys"] = best
    result["max_score"] = max_score
    result["best_time_for_roi"] = best_time

    # read submissions and fill what was submitted
    sent_buy_file = f"../best_buy/prob-{n:03}.buy"
    selected_buy = ""
    if os.path.isfile(sent_buy_file):
        selected_buy = read_file(sent_buy_file).strip('\n')

    if selected_buy == "":
        if result["gold"] < result["buys"][""]["time"]:
            selected_buy = "gold"

    result["selected"] = selected_buy

    # update ROI for each buy
    for _, item in result["buys"].items():
        update_roi(result["gold"], max_score, best_time, item)

    return result


if __name__ == "__main__":
    dump = []
    for i in range(1, 301):
        task_dump = process_task(i)
        dump.append(task_dump)

    with open('dump.json', 'w', encoding='utf-8') as outfile:
        json.dump(dump, outfile, ensure_ascii=False, sort_keys=True, indent=2)
