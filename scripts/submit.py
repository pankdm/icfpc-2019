#!/usr/bin/env python3

import argparse
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

    print(config)
