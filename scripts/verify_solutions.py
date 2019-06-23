#!/usr/bin/env python3

from api import verify_solution_file


PROBLEMS_DIR = "../problems/all/"
SOLUTIONS_DIR = "../solutions_cpp/"

for i in range(255, 256):
    problem_file = f"{PROBLEMS_DIR}prob-{i:03}.desc"
    solution_file = f"{SOLUTIONS_DIR}prob-{i:03}.sol"
    print(f"Verifying problem {i}")
    assert verify_solution_file(problem_file, solution_file)
