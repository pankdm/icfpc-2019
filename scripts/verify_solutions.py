from path import verify_solution


PROBLEMS_DIR = "../problems/all/"
SOLUTIONS_DIR = "../solutions_gold/"


for i in range(1, 301):
    problem_file = f"{PROBLEMS_DIR}prob-{i:03}.desc"
    solution_file = f"{SOLUTIONS_DIR}prob-{i:03}.sol"
    print (f"Verifying problem {i}")
    verify_solution(problem_file, solution_file)
