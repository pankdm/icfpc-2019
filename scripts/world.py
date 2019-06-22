
from common import (
    Item,
    Cell,
    FAST_WHEEL_STEPS,
    DRILL_STEPS,
)

class Boosters:
    def __init__(self, boosters):
        self.items = {}
        self.mistery = set()
        self.beacons = set()

        for pt, item in boosters.items():
            if item == Item.MYSTERY:
                self.mistery.add(pt)
            else:
                self.items[pt] = item

    def get_item(self, pt):
        return self.items.get(pt, None)

    def take_item(self, pt):
        assert pt in self.items
        del self.items[pt]

    def set_beacon(self, pt):
        assert pt not in self.beacons
        assert pt not in self.mistery
        self.beacons.add(pt)

    def assert_has_beacon(self, pt):
        assert pt in self.beacons

    def toList(self):
        result = []
        for k, v in self.items.items():
            result.append((v, k))
        for k in self.mistery:
            result.append(('X', k))
        return result


class Mappa:
    def __init__(self, contour, obstacles, location):
        self.contour = contour
        self.obstacles = obstacles
        self.init_box(contour)
        self.draw_contour(contour, Cell.EMPTY)
        for ob in obstacles:
            self.draw_contour(ob, Cell.BLOCK)

        for x in range(0, self.xmax):
            if self.get_color((x, 0)) == Cell.UNKNOWN:
                self.dfs_iter((x,0), Cell.BLOCK)
            if self.get_color((x, self.ymax-1)) == Cell.UNKNOWN:
                self.dfs_iter((x, self.ymax-1), Cell.BLOCK)

        for y in range(0, self.ymax):
            if self.get_color((0, y)) == Cell.UNKNOWN:
                self.dfs_iter((0, y), Cell.BLOCK)
            if self.get_color((self.xmax-1, y)) == Cell.UNKNOWN:
                self.dfs_iter((self.xmax-1, y), Cell.BLOCK)

        self.dfs_iter(location, Cell.EMPTY)

        for x in range(0, self.xmax):
            for y in range(0, self.ymax):
                if self.get_color((x, y)) == Cell.UNKNOWN:
                    self.set_color((x, y), Cell.BLOCK)

    def init_box(self, contour):
        # init the bounding box
        self.xmin = self.xmax = contour[0][0]
        self.ymin = self.ymax = contour[0][1]
        for (x, y) in contour:
            self.xmin = min(self.xmin, x)
            self.ymin = min(self.ymin, y)
            self.xmax = max(self.xmax, x)
            self.ymax = max(self.ymax, y)
        self.field = [[Cell.UNKNOWN] * self.ymax for i in range(self.xmax)]

    def return_view(self):
        res = [['z'] * self.ymax for i in range(self.xmax)]
        for x in range(0, self.xmax):
            for y in range(0, self.ymax):
                res[x][y] = cell_to_string(self.get_color((x, y)))
        return res


    def all_wrapped(self):
        for x in range(self.xmin, self.xmax):
            for y in range(self.ymin, self.ymax):
                if self.get_color((x, y)) == Cell.EMPTY:
                    return False
        return True

    def get_color(self, pt):
        return self.field[pt[0]][pt[1]]

    def set_color(self, pt, color):
        self.field[pt[0]][pt[1]] = color

    def is_in_bounds(self, pt):
        return (0 <= pt[0] < self.xmax and 0 <= pt[1] < self.ymax)

    def is_drillable(self, pt):
        return (self.xmin <= pt[0] < self.xmax and self.ymin <= pt[1] < self.ymax)

    def drill(self, pt):
        assert self.is_drillable(pt)
        # drill as wrapped
        self.set_color(pt, Cell.WRAPPED)

    def is_walkable(self, pt):
        if not self.is_in_bounds(pt):
            return False
        return self.get_color(pt) in [Cell.EMPTY, Cell.WRAPPED]

    def wrap(self, pt):
        if not self.is_in_bounds(pt):
            return False
        if self.get_color(pt) == Cell.EMPTY:
            self.set_color(pt, Cell.WRAPPED)

    # for large maps fails with
    # RecursionError: maximum recursion depth exceeded
    def dfs(self, pt, fill_color, visited):
        if not self.is_in_bounds(pt):
            return
        if self.get_color(pt) not in (Cell.UNKNOWN, fill_color):
            return
        if pt in visited:
            return

        self.set_color(pt, fill_color)
        visited.add(pt)
        for (dx, dy) in ((0, 1), (0, -1), (1, 0), (-1, 0)):
            self.dfs((pt[0]+dx, pt[1]+dy), fill_color, visited)

    def dfs_iter(self, pt, fill_color):
        stack = [pt]
        visited = set()
        while len(stack) > 0:
            pt = stack.pop()
            if not (0 <= pt[0] < self.xmax and 0 <= pt[1] < self.ymax):
                continue
            if self.get_color(pt) not in (Cell.UNKNOWN, fill_color):
                continue
            if pt in visited:
                continue

            self.set_color(pt, fill_color)
            visited.add(pt)
            for (dx, dy) in ((0, 1), (0, -1), (1, 0), (-1, 0)):
                stack.append((pt[0]+dx, pt[1]+dy))
            visited.add(pt)

    def draw_contour(self, contour, inside_color):
        if contour is None:
            return
        assert(len(contour) >= 4)
        prev = contour[0]
        for pt in contour[1:] + [prev]:
            assert(prev != pt)
            assert(prev[0] == pt[0] or prev[1] == pt[1])
            if prev[0] < pt[0]:
                for i in range(prev[0], pt[0]):
                    self.field[i][prev[1]] = inside_color
            elif prev[0] > pt[0]:
                for i in range(pt[0], prev[0]):
                    self.field[i][prev[1]-1] = inside_color
            elif prev[1] < pt[1]:
                for i in range(prev[1], pt[1]):
                    self.field[prev[0]-1][i] = inside_color
            else: # prev[1] > pt[1]:
                for i in range(pt[1], prev[1]):
                    self.field[prev[0]][i] = inside_color
            prev = pt


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
            self.arms.append(new_arm)

class Action:
    def __init__(self, type, pt = None):
        self.type = type
        self.pt = pt

class World:
    # location: type Point
    # mappa: type Mappa
    # boosters: type Boosters
    def __init__(self, location, mappa, boosters):
        self.steps = 0
        self.mappa = mappa

        self.boosters = boosters

        self.x, self.y = location
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

        # count boosters
        self.num_fast_wheels = 0
        self.num_drills = 0
        self.num_manipulators = 0
        self.num_teleports = 0
        self.num_clones = 0

        self._wrap()

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

        if self.mappa.is_walkable(pt):
            move_ok = True
        elif self.remaining_drill > 0:
            if self.mappa.is_drillable(pt):
                move_ok = True

        if not move_ok:
            return

        # drill anyway
        self.mappa.drill(pt)
        self.x = x
        self.y = y

        item = self.boosters.get_item(pt)
        if item:
            self._handle_item(x, y, item)

    def _handle_item(self, x, y, item):
        if item == Item.MYSTERY:
            # nothing happens
            return
        self.boosters.take_item((x, y))
        if item == Item.MANIPULATOR:
            self.num_manipulators += 1
        elif item == Item.WHEEL:
            self.num_fast_wheels += 1
        elif item == Item.DRILL:
            self.num_drills += 1
        elif item == Item.TELEPORT:
            self.num_teleports += 1
        elif item == Item.CLONE:
            self.num_clones += 1
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

    def set_beacon(self):
        self.boosters.set_beacon((self.x, self.y))
        self._on_step_finish()

    def shift(self, x, y):
        self.boosters.assert_has_beacon((x, y))
        self.x = x
        self.y = y
        self._on_step_finish()

    # private API:
    def _wrap(self):
        for arm in self.manipulators.arms:
            x = self.x + arm.x
            y = self.y + arm.y
            # TODO: check visibility
            self.mappa.wrap((x, y))

    def all_wrapped(self):
        return self.mappa.all_wrapped()

    def debug_print(self):
        view = self.mappa.return_view()
        for arm in self.manipulators.arms:
            x = self.x + arm.x
            y = self.y + arm.y
            if 0 <= x < len(view) and 0 <= y <= len(view[x]):
                view[x][y] = 'x'
        view[self.x][self.y] = '/'

        for pt, booster in self.boosters.items():
            x, y = pt
            view[x][y] = booster

        for y in reversed(range(0, len(view[0]))):
            for x in range(0, len(view)):
                print (view[x][y],)
            print ("")


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
        world.set_beacon()
    elif t == 'T':
        x, y = action.pt
        world.shift(x, y)
    else:
        assert False, "Unknown action: {}".format(t)

# if __name__ == "__main__":
#     PART1_DIRECTORY = "../problems/part-1-examples/"
#     problem = PART1_DIRECTORY + 'example-01.desc'
#     solution1 = PART1_DIRECTORY + 'example-01-1.sol'
#     assert verify_solution_file(problem, solution1)
