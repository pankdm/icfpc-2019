
from checker import (
    parse_problem,
    Item
)

FAST_WHEEL_STEPS = 50
DRILL_STEPS = 30

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
        for arm in self.arms:
            arm.rotate_clockwise()

    def rotate_counter_clockwise(self):
        for arm in self.arms:
            arm.rotate_counter_clockwise()

    def add_arm(self, dx, dy):
        new_arm = Arm(dx, dy)
        for arm in self.arms:
            if arm == new_arm:
                # cannot add existing arm
                return

        ok = False
        for arm in self.arms:
            if abs(arm.x - new_arm.x) + abs(arm.y - new_arm.y) == 1:
                ok = True
                break

        if ok:
            self.arms.append(arm)


class Action:
    def __init__(self, type, pt = None):
        self.type = type
        self.pt = pt

class World:
    def __init__(self, desc):
        self.steps = 0
        self.desc = desc

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

        self.remaining_fast_wheels = 0
        self.remaining_drill = 0

        self.num_fast_wheels = 0
        self.num_drills = 0
        self.num_manipulators = 0
        self.num_teleports = 0

    def _on_step_finish(self):
        self._wrap();

        self.steps += 1
        if self.remaining_fast_wheels > 0:
            self.remaining_fast_wheels -= 1
        if self.remaining_drill > 0:
            self.remaining_drill -= 1

    # public API:
    def do_nothing(self):
        self._on_step_finish()

    def move(self, dx, dy):
        self._move_impl(dx, dy)
        if self.remaining_fast_wheels > 0:
            # extra move, but need to wrap before
            self._wrap()
            self._move_impl(dx, dy)
        # TODO: support drill
        self._on_step_finish()


    def _move_impl(self, dx, dy):
        x = self.x + dx
        y = self.y + dy
        pt = (x, y)
        move_ok = False

        if self.desc.is_walkable(pt):
            move_ok = True
        elif self.remaining_drill > 0:
            if self.desc.is_drillable(pt):
                move_ok = True

        if not move_ok:
            return

        # drill anyway
        self.desc.drill(pt)
        self.x = x
        self.y = y

        item = self.desc.get_item(pt)
        if item:
            self._handle_item(x, y, item)


    def _handle_item(self, x, y, item):
        if item == Item.MYSTERY:
            # nothing happens
            return
        self.desc.take_item((x, y))
        if item == Item.MANIPULATOR:
            self.num_manipulators += 1
        elif item == Item.WHEEL:
            self.num_fast_wheels += 1
        elif item == Item.DRILL:
            self.num_drills += 1
        elif item == Item.TELEPORT:
            self.num_teleports += 1
        else:
            assert False, "Unsupported item: {}".format(item)


    def rotate_clockwise(self):
        self.manipulators.rotate_clockwise()
        self._on_step_finish()

    def rotate_counter_clockwise(self):
        self.manipulators.rotate_counter_clockwise()
        self._on_step_finish()

    def add_new_manipulator(self, dx, dy):
        self.manipulators.add_arm(dx, dy)
        self._on_step_finish()

    def fast_wheels(self):
        self.remaining_fast_wheels = FAST_WHEEL_STEPS
        self._on_step_finish()

    def start_drill(self):
        self.remaining_drill = DRILL_STEPS
        self._on_step_finish()

    def set_teleport(self):
        assert False
        self._on_step_finish()

    def shift(self, x, y):
        assert False
        self._on_step_finish()

    # private API:
    def _wrap(self):
        for arm in self.manipulators.arms:
            x = self.x + arm.x
            y = self.y + arm.y
            # TODO: check visibility
            self.desc.wrap((x, y))

    def all_wrapped(self):
        return self.desc.all_wrapped()

    def debug_print(self):
        view = self.desc.return_view()
        for arm in self.manipulators.arms:
            x = self.x + arm.x
            y = self.y + arm.y
            view[x][y] = 'x'
        view[self.x][self.y] = '/'

        for pt, booster in self.desc.boosters.items():
            x, y = pt
            view[x][y] = booster

        for y in reversed(range(0, len(view[0]))):
            for x in range(0, len(view)):
                print view[x][y],
            print ""



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
    s = f.read()
    f.close()
    return s

def verify_solution(problem_file, solution_file):
    problem = read_file(problem_file)
    solution = read_file(solution_file)

    desc = parse_problem(problem)
    world = World(desc)

    actions = parse_solution(solution)
    for action in actions:
        print 'applying action', action.type, action.pt
        apply_action(world, action)
        world.debug_print()
        raw_input(">")

    assert world.steps == len(actions)
    assert world.all_wrapped()


if __name__ == "__main__":
    PART1_DIRECTORY = "../problems/part-1-examples/"
    problem = PART1_DIRECTORY + 'example-01.desc'
    solution1 = PART1_DIRECTORY + 'example-01-1.sol'
    verify_solution(problem, solution1)
