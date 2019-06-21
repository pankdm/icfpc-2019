#!/usr/bin/env python3

from yaml import safe_load as yaml

if __name__ == "__main__":
    file = open("config.yaml", "r")
    config = yaml(file)
    print(config)
