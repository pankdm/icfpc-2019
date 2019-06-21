#!/usr/bin/env python3

import argparse
import os
import sys
from yaml import safe_load as yaml

if __name__ == "__main__":
    with open("config.yaml", "r") as config_file:
        config = yaml(config_file)

    parser = argparse.ArgumentParser(argument_default=argparse.SUPPRESS)
    parser.add_argument("-g", "--gold", help="Directory with gold solutions")
    parser.add_argument("-e", "--evaluate",
                        help="Directory with solutions to evaluate")
    parser.add_argument("-p", "--problems", nargs="*",
                        help="List of directories containing problems")
    args = parser.parse_args()

    config.update(vars(args))

    if not config["problems"]:
        sys.exit("No problems specified")

    problems = []
    for dir in config["problems"]:
        if os.path.exists(dir):
            files = os.listdir(dir)
            descriptions = filter(
                lambda f: os.path.splitext(f)[-1] == ".desc", files)
            problems.extend(descriptions)

    if not problems:
        sys.exit(f'No problems found in directories {config["problems"]}')
    else:
        print(f'Found {len(problems)} problem(s)')

    try:
        os.makedirs(config["gold"], exist_ok=True)
    except:
        sys.exit("No gold directory exists, and unable to create")

    if not config["evaluate"] or not os.path.exists(config["evaluate"]):
        sys.exit("No new problems for evaluation")

    # TODO: evaluation
    # TODO: submit
    # TODO: merge gold solutions
