
from checker import (parse_problem)

class Arm:
    def __init__(self, x, y):
        # x and y are relative
        self.x = x
        self.y = y

    def rotate_clockwise(self):
        self.x, self.y = self.y, -self.x

    def rotate_counter_clockwise(self):
        self.x, self.y = -self.y, self.x

    def __eq__(self, other):
        return self.x == other.x and self.y == other.y

class Manipulators:
    def __init__(self, arms):
        self.arms = arms

    def rotate_clockwise(self):
        for arm in arms:
            arm.rotate_clockwise()

    def rotate_counter_clockwise(self):
        for arm in arms:
            arm.rotate_counter_clockwise()

class Action:
    def __init__(self, type, pt = None):
        self.type = type
        self.pt = pt

class World:
    def __init__(self, desc):
        self.step = 0
        self.field = desc

        self.x, self.y = desc.loc
        arms = [
            Arm(1, 0),
            Arm(1, 1),
            Arm(1, -1),
            # for simplicity,
            # consider the robot itself to be arm
            Arm(0, 0)
        ]
        self.manipulators = Manipulators(arms)

    # public API:
    def do_nothing(self):
        self.step += 1

    def move(self, dx, dy):
        self.step += 1

    def rotate_clockwise(self):
        self.step += 1
        self.manipulators.rotate_clockwise()
        # TODO: wrap

    def rotate_counter_clockwise(self):
        self.step += 1
        self.manipulators.rotate_counter_clockwise()
        # TODO: wrap

    def add_new_manipulator(self, dx, dy):
        pass

    def fast_wheels(self):
        pass

    def start_drill(self):
        pass

    def set_teleport(self):
        pass

    def shift(self, x, y):
        pass

    # private API:
    

def apply_action(world, action):
    t = action.type
    if t == 'W':
        world.move(0, 1)
    elif t == 'S':
        world.move(0, -1)
    elif t == 'A':
        world.move(-1, 0)
    elif t == 'D':
        world.move(1, 0)
    elif t == 'Z':
        world.do_nothing()
    elif t == 'E':
        world.rotate_clockwise()
    elif t == 'Q':
        world.rotate_counter_clockwise()
    elif t == 'B':
        dx, dy = action.pt
        world.add_new_manipulator(dx, dy)
    elif t == 'F':
        world.fast_wheels()
    elif t == 'L':
        world.start_drill()
    elif t == 'R':
        world.set_teleport()
    elif t == 'T':
        x, y = action.pt
        world.shift(x, y)
    else:
        assert False, "Unknown action: {}".format(t)

# returns pair of next index, result
def parse_token(path, index):
    assert index < len(path)
    single = set(['W', 'S', 'A', 'D', 'Z', 'E', 'Q', 'F', 'L'])
    with_pt = set(['B', 'T'])
    if path[index] in single:
        action = Action(path[index])
        return (action, index + 1)
    if path[index] in with_pt:
        assert path[index + 1] == '('

        index1 = path.find(',', index + 1)
        assert index1 > 0
        assert path[index1] == ','
        x = int(path[index + 2 : index1])

        index2 = path.find(')', index1 + 1)
        assert index2 > 0
        assert path[index2] == ')'
        y = int(path[index1 + 1 : index2])

        action = Action(path[index])
        action.pt = (x, y)
        return action, index2 + 1
    assert False, 'Unknown action: {}'.format(path[index])

def parse_solution(solution):
    path = solution.strip('\n')
    index = 0
    res = []
    while True:
        action, new_index = parse_token(path, index)

        assert new_index > index
        index = new_index
        res.append(action)
        if index >= len(path):
            return res


def read_file(file_name):
    f = open(file_name)
    return f.read()

def evaluate_solution(problem_file, solution_file):
    problem = read_file(problem_file)
    solution = read_file(solution_file)

    desc = parse_problem(problem)
    world = World(desc)

    actions = parse_solution(solution)
    for action in actions:
        apply_action(world, action)


if __name__ == "__main__":
    directory = "../problems/part-1-examples/"
    evaluate_solution(directory + "example-01.desc", directory + "example-01-1.sol")
