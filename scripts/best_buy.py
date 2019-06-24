#!/usr/bin/env python3

import sys

from argparse import ArgumentParser, SUPPRESS
import csv
import hashlib
import os
import re
import time
from types import SimpleNamespace
from zipfile import ZipFile, ZIP_DEFLATED

from bs4 import BeautifulSoup
import requests
from tqdm import tqdm, trange
from yaml import safe_load, dump

from parser import parse_problem, parse_solution
from api import read_file, verify_solution

SERVER = "https://monadic-lab.org"


class KnapsackItem:
    def __init__(self):
        self.roi = None
        self.deltaroi = None
        self.spent = None
        self.problem = None
        self.path = None
        self.files = None

    def __str__(self):
        return f"Item({self.roi:.2f}, {self.deltaroi:.2f}, {self.spent}, {self.problem}, {self.path}, {self.files})"

    def to_short_string(self):
        return f"Item({self.roi:.2f}, {self.deltaroi:.2f}, {self.spent}, {self.problem})"


def solve_knapsack(balance, knapsack):
    knapsack.sort(key=lambda x: x.deltaroi, reverse=True)
    best_buy = {}
    for item in knapsack:
        if item.spent > balance:
            continue
        if item.roi <= 0:
            continue
        if item.deltaroi <= 0:
            continue

        if item.problem in best_buy:
            if best_buy[item.problem].spent > item.spent:
                continue
            print(
                f"Updating solution for {item.problem}, cost {best_buy[item.problem].spent} → {item.spent}")
            balance += best_buy[item.problem].spent

        prefix = item.path.replace(".meta.yaml", "")
        for ext in [".sol", ".buy"]:
            name = prefix + ext
            if os.path.exists(name):
                item.files.append((name, f"prob-{item.problem:03}{ext}"))

        print(f"{item.roi:.2f}    taking item {item.to_short_string()}")
        best_buy[item.problem] = item
        balance -= item.spent

    print(f"Remaining balance {balance}")
    return best_buy


if __name__ == "__main__":
    with open("best_buy.yaml", "r") as config_file:
        yaml_config = safe_load(config_file)

    parser = ArgumentParser(argument_default=SUPPRESS)
    parser.add_argument("-g", "--gold", help="Directory with gold solutions")
    parser.add_argument("-b", "--buydb",
                        help="Directory with database for purchase-based solutions")
    parser.add_argument("-l", "--lambdaclient",
                        help="Directory with lambda-client")
    parser.add_argument("-m", "--merge",
                        help="Directory to store best known combination of solutions")
    parser.add_argument("-t", "--token", help="Team private ID")

    args = parser.parse_args()

    yaml_config.update(vars(args))

    config = SimpleNamespace(**yaml_config)

    balance_file = os.path.join(config.lambdaclient, "balance.txt")

    if not os.path.exists(balance_file):
        sys.exit("No balance present")

    if not os.path.exists(config.gold):
        sys.exit("No gold solutions present")

    with open(balance_file, "r") as f:
        balance = int(f.read())

    print(f"Distributing {balance} shitcoins")

    knapsack = []

    for task_dirname in os.listdir(config.buydb):
        task_dir = os.path.join(config.buydb, task_dirname)
        if not os.path.isdir(task_dir):
            continue
        problem = re.search("^task(\d+)$", task_dirname, re.IGNORECASE)
        if not problem:
            continue
        problem = int(problem.group(1))

        gold_name = os.path.join(
            config.gold, f"prob-{problem:03}.sol")
        gold_metadata_name = os.path.join(
            config.gold, f"prob-{problem:03}.meta.yaml")

        if not os.path.exists(gold_name):
            sys.exit("No gold solution for problem {task_num}")

        gold_metadata = {}
        if os.path.exists(gold_metadata_name):
            with open(gold_metadata_name, "r") as gold_metadata_file:
                gold_metadata = safe_load(gold_metadata_file)

        if "time" in gold_metadata:
            gold_score = gold_metadata["time"]
        else:
            gold_content = read_file(gold_name)
            gold_score = len(parse_solution(gold_content))

        metadata = list(filter(
            lambda x: x.endswith(".meta.yaml"), os.listdir(task_dir)))

        absolute_roi_map = {}
        absolute_roi_map[0] = (gold_name, 0, gold_score, 0)

        for metadata_name in metadata:
            path = os.path.join(task_dir, metadata_name)

            spent, score, max_score = 0, gold_score, 0
            with open(path, "r") as f:
                yaml = safe_load(f)
                if not yaml:
                    print(f"{path} borked")
                    continue
                spent, score, max_score = yaml["spent"], yaml["time"], yaml["max_score"]

            if spent != 0:
                if spent not in absolute_roi_map or absolute_roi_map[spent][2] > score:
                    absolute_roi_map[spent] = (path, spent, score, max_score)

        absolute_roi = []
        for _, data in absolute_roi_map.items():
            absolute_roi.append(data)

        absolute_roi.sort(key=lambda x: x[1])

        gold_score = absolute_roi[0][2]
        prev = absolute_roi[0]

        for curr in absolute_roi[1:]:
            path, spent, score, max_score = curr
            _, prev_spent, prev_score, _ = prev
            delta = (max_score * (prev_score - score) / prev_score)
            spent_delta = spent - prev_spent
            deltaroi = (delta - spent_delta) / spent_delta

            gold_delta = (max_score * (gold_score - score) / gold_score)
            roi = (gold_delta - spent) / spent

            if deltaroi < 0 or roi < 0:
                continue

            item = KnapsackItem()
            item.roi = roi
            item.deltaroi = deltaroi
            item.spent = spent
            item.problem = problem
            item.path = path
            item.files = []
            knapsack.append(item)

            prev = curr

    best_buy = solve_knapsack(balance, knapsack)
    best_buy_files = {}

    golds = list(filter(lambda x: x.endswith(".sol"), os.listdir(config.gold)))
    for gold in golds:
        problem = re.search("^prob-(\d+)\.sol$", gold, re.IGNORECASE)
        if not problem:
            continue
        problem = int(problem.group(1))
        if problem in best_buy:
            best_buy_files[problem] = best_buy[problem].files
            continue

        print(f"Filling problem {problem} from gold {gold}")
        gold_name = os.path.join(config.gold, gold)
        prefix = gold_name.replace(".sol", "")
        files = []
        for ext in [".sol", ".buy"]:
            name = prefix + ext
            if os.path.exists(name):
                files.append((name, f"prob-{problem:03}{ext}"))
        best_buy_files[problem] = files

    submissions = []
    for _, files in best_buy_files.items():
        submissions.extend(files)

    try:
        os.makedirs(config.submission, exist_ok=True)
    except:
        sys.exit("No submission directory exists, and unable to create")

    try:
        os.makedirs(config.merge, exist_ok=True)
    except:
        sys.exit("No gold merge directory exists, and unable to create")

    print(f"Submitting new solutions")
    # sys.exit("early exit")

    timestamp = time.strftime("%Y-%m-%d_%H-%M-%S")
    archive = os.path.join(config.submission, timestamp + ".zip")
    with ZipFile(archive, mode="w", compression=ZIP_DEFLATED, compresslevel=9) as zipfile:
        for submission, zipname in tqdm(submissions, desc="Archiving new solutions"):
            zipfile.write(submission, arcname=zipname)

    if config.token:
        with open(archive, mode="rb") as zipfile:
            data = {"private_id": config.token}
            response = requests.post(f"{SERVER}/submit",
                                     data=data, files={"file": zipfile})
            if response.status_code == 429:
                soup = BeautifulSoup(response.text, "html.parser")
                error = soup.html.body.ul.li.span
                sys.exit(error)
            elif response.status_code != 200:
                sys.exit(
                    f"Error {response.status_code} while submitting solutions")

        soup = BeautifulSoup(response.text, "html.parser")
        li1, li2, li3 = soup.html.body.ul.find_all("li")
        remote_zip = li1.samp.string
        remote_hash = li2.samp.string
        remote_folder = li3.a.string

        result_search = re.search(
            "^team_[\d]+_(.*)\.zip$", remote_zip, re.IGNORECASE)
        if result_search:
            result_folder = result_search.group(1)
            result_url = f"{SERVER}{remote_folder}{result_folder}/score.csv"
            print(f"Awaiting for result at {result_url}")
            for _ in trange(60):
                # Busy loop here, it takes time to produce solution anyway
                time.sleep(3)
                scores_data = requests.get(result_url)
                if scores_data.status_code == 200:
                    break

            if (scores_data.status_code != 200):
                sys.exit("Awaiting for results timed out, please update manually")
            scores_path = os.path.join(config.submission, timestamp + ".csv")
            with open(scores_path, "w") as scores_file:
                scores_file.write(scores_data.text)

            scores = [row for row in csv.reader(
                scores_data.text.split("\n"), skipinitialspace=True) if row]

            update_archive = False
            for (problem, score, status) in scores:
                problem = int(problem)
                score = int(score)
                solution_name = f"prob-{problem:03}.sol"
                buy_name = f"prob-{problem:03}.buy"
                merge = os.path.join(config.merge, solution_name)
                merge_buy = os.path.join(config.merge, buy_name)
                if status != "Ok":
                    print(f"Problem {problem} failed with status {status}")
                    continue
                metadata_name = f"prob-{problem:03}.meta.yaml"
                merge_metadata_name = os.path.join(config.merge, metadata_name)
                merge_metadata = {}
                if os.path.exists(merge_metadata_name):
                    with open(merge_metadata_name, "r") as merge_metadata_file:
                        merge_metadata = safe_load(merge_metadata_file)

                if "time" not in merge_metadata or merge_metadata["time"] > score:
                    print(f"Problem {problem} solution improved")
                    update_archive = True
                    for from_name, to_name in best_buy_files[problem]:
                        with open(from_name, "rb") as src, open(os.path.join(config.merge, to_name), "wb") as dst:
                            dst.write(src.read())
                    with open(merge, "rb") as src:
                        data = src.read()
                        md5 = hashlib.md5(data).hexdigest()
                    with open(merge_metadata_name, "w") as merge_metadata_file:
                        metadata = {
                            "time": score,
                            "hash": md5
                        }
                        dump(metadata, merge_metadata_file)
            if update_archive:
                archive_path = os.path.join(config.merge, "bestbuy.zip")
                merge_files = os.listdir(config.merge)
                merge_solutions = filter(
                    lambda f: os.path.splitext(f)[-1] in [".sol", ".buy"], merge_files)
                with ZipFile(archive_path, mode="w", compression=ZIP_DEFLATED, compresslevel=9) as zipfile:
                    for solution in tqdm(merge_solutions, desc="Archiving gold solutions"):
                        zipname = os.path.basename(solution)
                        filename = os.path.join(config.merge, solution)
                        zipfile.write(filename, arcname=zipname)
    else:
        sys.exit(f"Private id is not set, you can submit {archive} manually")
