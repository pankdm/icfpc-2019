#!/usr/bin/env python3

from argparse import ArgumentParser, SUPPRESS
import os
import sys
from types import SimpleNamespace

from tqdm import tqdm
from yaml import safe_load

if __name__ == "__main__":
    with open("config.yaml", "r") as config_file:
        yaml_config = safe_load(config_file)

    parser = ArgumentParser(argument_default=SUPPRESS)
    parser.add_argument("-g", "--gold", help="Directory with gold solutions")
    parser.add_argument("-e", "--evaluate",
                        help="Directory with solutions to evaluate")
    parser.add_argument("-p", "--problems", nargs="*",
                        help="List of directories containing problems")
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

    submissions = []
    for problem in tqdm(problems, desc="Comparing solutions"):
        problem_name = os.path.basename(problem)
        solution_name = os.path.splitext(problem_name)[0] + ".sol"
        gold = os.path.join(config.gold, solution_name)
        evaluate = os.path.join(config.evaluate, solution_name)
        # TODO: proper evaluation
        if os.path.exists(evaluate) and not os.path.exists(gold):
            print(f"New solution for {problem_name}")
            submissions.append(evaluate)

    print(f"Submitting {len(submissions)} new solutions")

    # TODO: submit
    # TODO: merge gold solutions
