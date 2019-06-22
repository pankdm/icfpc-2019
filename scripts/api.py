from parser import (
    parse_problem,
    parse_solution,
)

from world import (
    World,
    apply_action,
)


def read_file(file_name):
    f = open(file_name)
    s = f.read()
    f.close()
    return s


def verify_solution_file(problem_file, solution_file):
    problem = read_file(problem_file)
    solution = read_file(solution_file)

    return verify_solution(problem, solution)


def verify_solution(problem, solution):
    world = parse_problem(problem)
    actions = parse_solution(solution)
    for action in actions:
        # print 'applying action', action.type, action.pt
        apply_action(world, action)
        # world.debug_print()
        # raw_input(">")

    return world.steps == len(actions) and world.all_wrapped()
