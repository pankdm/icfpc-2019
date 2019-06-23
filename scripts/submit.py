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

if __name__ == "__main__":
    with open("config.yaml", "r") as config_file:
        yaml_config = safe_load(config_file)

    parser = ArgumentParser(argument_default=SUPPRESS)
    parser.add_argument("-g", "--gold", help="Directory with gold solutions")
    parser.add_argument("-e", "--evaluate",
                        help="Directory with solutions to evaluate")
    parser.add_argument("-p", "--problems", nargs="*",
                        help="List of directories containing problems")
    parser.add_argument("-s", "--submission",
                        help="Directory with all existing submissions")
    parser.add_argument("-t", "--token", help="Team private ID")
    parser.add_argument("-v", "--verify", action='store_true',
                        help="Enable submission verification")
    parser.add_argument("-b", "--best", action='store_true',
                        help="Submit best solution known (requires solution parsing)")
    args = parser.parse_args()

    yaml_config.update(vars(args))

    config = SimpleNamespace(**yaml_config)

    if not config.problems:
        sys.exit("No problems specified")

    problems = []
    for dir in config.problems:
        if os.path.exists(dir):
            files = os.listdir(dir)
            descriptions = filter(
                lambda f: os.path.splitext(f)[-1] == ".desc", files)
            descriptions = map(
                lambda f: os.path.join(dir, f), descriptions)
            problems.extend(descriptions)

    if not problems:
        sys.exit(f"No problems found in directories {config.problems}")
    else:
        print(f"Found {len(problems)} problem(s)")

    try:
        os.makedirs(config.gold, exist_ok=True)
    except:
        sys.exit("No gold directory exists, and unable to create")

    if not config.evaluate or not os.path.exists(config.evaluate):
        sys.exit("No new problems for evaluation")

    try:
        os.makedirs(config.submission, exist_ok=True)
    except:
        sys.exit("No submission directory exists, and unable to create")

    submissions = []
    new_submissions = 0
    with tqdm(problems, desc="Comparing solutions") as progress:
        for problem in progress:
            problem_name = os.path.basename(problem)
            progress.set_description(problem_name)
            problem_prefix = os.path.splitext(problem_name)[0]
            solution_name = problem_prefix + ".sol"
            buy_name = problem_prefix + ".buy"
            gold = os.path.join(config.gold, solution_name)
            gold_buy = os.path.join(config.gold, buy_name)
            gold_metadata_name = os.path.join(
                config.gold, problem_prefix + ".meta.yaml")
            evaluate = os.path.join(config.evaluate, solution_name)
            evaluate_buy = os.path.join(config.evaluate, buy_name)

            should_evaluate = False
            if os.path.exists(evaluate):
                problem_content = read_file(problem)
                solution_content = read_file(evaluate)
                if config.verify and not verify_solution(problem_content, solution_content):
                    print(
                        f"Evaluated solution for {problem_name} is incorrect")
                elif config.best and os.path.exists(gold):
                    gold_metadata = {}
                    if os.path.exists(gold_metadata_name):
                        with open(gold_metadata_name, "r") as gold_metadata_file:
                            gold_metadata = safe_load(gold_metadata_file)

                    if "time" in gold_metadata:
                        gold_time = gold_metadata["time"]
                    else:
                        gold_content = read_file(gold)
                        gold_time = len(parse_solution(gold_content))

                    new_time = len(parse_solution(solution_content))

                    if new_time < gold_time:
                        should_evaluate = True
                else:
                    should_evaluate = True

            if should_evaluate:
                submissions.append(evaluate)
                if os.path.exists(evaluate_buy):
                    submissions.append(evaluate_buy)
                new_submissions += 1
            elif os.path.exists(gold):
                submissions.append(gold)
                if os.path.exists(gold_buy):
                    submissions.append(gold_buy)
            else:
                print(f"No solution for {problem_name}")

    print(f"Submitting {new_submissions} new solutions")
    timestamp = time.strftime("%Y-%m-%d_%H-%M-%S")
    archive = os.path.join(config.submission, timestamp + ".zip")
    with ZipFile(archive, mode="w", compression=ZIP_DEFLATED, compresslevel=9) as zipfile:
        for submission in tqdm(submissions, desc="Archiving new solutions"):
            zipname = os.path.basename(submission)
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
                gold = os.path.join(config.gold, solution_name)
                evaluate = os.path.join(config.evaluate, solution_name)
                gold_buy = os.path.join(config.gold, buy_name)
                evaluate_buy = os.path.join(config.evaluate, buy_name)
                if evaluate not in submissions:
                    # This solution was skipped, ignore the submission
                    continue
                if status != "Ok":
                    print(f"Problem {problem} failed with status {status}")
                    continue
                metadata_name = f"prob-{problem:03}.meta.yaml"
                gold_metadata_name = os.path.join(config.gold, metadata_name)
                gold_metadata = {}
                if os.path.exists(gold_metadata_name):
                    with open(gold_metadata_name, "r") as gold_metadata_file:
                        gold_metadata = safe_load(gold_metadata_file)

                if "time" not in gold_metadata or gold_metadata["time"] > score:
                    print(f"Problem {problem} solution improved")
                    archive = True
                    with open(evaluate, "rb") as src, open(gold, "wb") as dst:
                        data = src.read()
                        md5 = hashlib.md5(data).hexdigest()
                        dst.write(data)
                    if os.path.exists(evaluate_buy):
                        with open(evaluate_buy, "rb") as src, open(gold_buy, "wb") as dst:
                            data = src.read()
                            dst.write(data)
                    with open(gold_metadata_name, "w") as gold_metadata_file:
                        metadata = {
                            "time": score,
                            "hash": md5
                        }
                        dump(metadata, gold_metadata_file)
            if update_archive:
                archive_path = os.path.join(config.gold, "gold.zip")
                gold_files = os.listdir(config.gold)
                gold_solutions = filter(
                    lambda f: os.path.splitext(f)[-1] in [".sol", ".buy"], gold_files)
                with ZipFile(archive_path, mode="w", compression=ZIP_DEFLATED, compresslevel=9) as zipfile:
                    for solution in tqdm(gold_solutions, desc="Archiving gold solutions"):
                        zipname = os.path.basename(solution)
                        zipfile.write(solution, arcname=zipname)
    else:
        sys.exit(f"Private id is not set, you can submit {archive} manually")
