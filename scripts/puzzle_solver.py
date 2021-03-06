#!/usr/bin/env python3

from PIL import Image

from puzzle_parser import PuzzleSpec
from parser import read_file, parse_problem, write_problem, TaskSpec
from puzzle_validator import puzzle_valid

import sys


from copy import deepcopy
from collections import deque
from random import randint, shuffle

from world import World, Mappa


def rotate_clockwise(vec):
    x, y = vec
    return (y, -x)


def rotate_counter_clockwise(vec):
    x, y = vec
    return (-y, x)


def next_point(pt, vec):
    x, y = pt
    dx, dy = vec
    return (x + dx, y + dy)


def prev_point(pt, vec):
    x, y = pt
    dx, dy = vec
    return (x - dx, y - dy)


def next_corner(pt, vec, new_vec):
    x, y = pt
    dx, dy = 0, 0
    if vec == (1, 0) and new_vec == (0, 1):
        dx, dy = 1, 0
    elif vec == (1, 0) and new_vec == (0, -1):
        dx, dy = 0, 0
    elif vec == (0, 1) and new_vec == (1, 0):
        dx, dy = 1, 0
    elif vec == (0, 1) and new_vec == (-1, 0):
        dx, dy = 1, 1
    elif vec == (-1, 0) and new_vec == (0, 1):
        dx, dy = 1, 1
    elif vec == (-1, 0) and new_vec == (0, -1):
        dx, dy = 0, 1
    elif vec == (0, -1) and new_vec == (1, 0):
        dx, dy = 0, 0
    elif vec == (0, -1) and new_vec == (-1, 0):
        dx, dy = 0, 1
    return (x + dx, y + dy)


class State:
    UNKNOWN = 0
    GOOD = 1
    BAD = 2
    FILLED = 3
    CONNECTED = 4
    CONTOUR = 5


def get_color(state):
    colors = {
        State.UNKNOWN: (0, 0, 0),  # black
        State.GOOD: (0, 255, 0),  # green
        State.BAD: (255, 0, 0),  # red
        State.FILLED: (255, 255, 0),  # yellow
        State.CONNECTED: (255, 0, 0),  # red
        State.CONTOUR: (128, 128, 128),  # grey
    }
    return colors[state]


def manhattan(pt, size):
    x, y = pt
    return min([x, y, size - 1 - x, size - 1 - y])


class PuzzleSolver:
    def __init__(self, spec, draw):
        self.draw = draw
        self.spec = spec
        self.size = spec.tSize
        self.field = [[State.UNKNOWN] * self.size for x in range(self.size)]

        print(f"size = {self.size}, min, max: {spec.vMin}, {spec.vMax}, incl: %d, excl: %d" % (len(spec.included), len(spec.excluded)))

        for pt in spec.included:
            x, y = pt
            self.field[x][y] = State.GOOD

        shuffle(spec.excluded)

        for pt in spec.excluded:
            x, y = pt
            self.field[x][y] = State.BAD

        self.world = None
        self.used_for_boosters = set()
        self.used_for_something = set()

        self.counter = 0

    def get_points_order(self):
        pts = [pt for pt in spec.excluded]
        # pts = list(filter(lambda p: p[0] and p[1], pts))
        pts.sort(key=lambda x: manhattan(x, self.size))
        return pts

    def bfs_to_filled(self, start):
        q = deque()
        q.append(start)

        dist = {}
        dist[start] = 0
        back = {}

        target = None

        while q:
            now = q.popleft()
            d = dist[now]

            x0, y0 = now
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                x = x0 + dx
                y = y0 + dy
                next = (x, y)
                if self.get_state((x, y)) in [State.FILLED, State.CONNECTED]:
                    target = now
                    break

                if self.get_state((x, y)) == State.UNKNOWN:
                    if next not in dist:
                        dist[next] = d + 1
                        back[next] = now
                        q.append(next)

            if target:
                break

        # do backwards pass
        while target != start:
            x, y = target
            assert self.get_state((x, y)) == State.UNKNOWN
            self.field[x][y] = State.FILLED
            target = back.get(target, None)

        self.set_state(start, State.CONNECTED)

    def set_state(self, pt, state):
        x, y = pt
        self.field[x][y] = state

    def get_state(self, pt):
        x, y = pt
        # virtual border of FILLED around
        if x < 0: return State.FILLED
        if y < 0: return State.FILLED
        if x >= self.size: return State.FILLED
        if y >= self.size: return State.FILLED
        return self.field[x][y]

    def rand_point(self):
        x = randint(0, self.spec.tSize - 1)
        y = randint(0, self.spec.tSize - 1)
        return (x, y)

    def gen_booster_point(self, mappa):
        while True:
            res = self.rand_point()
            if (res not in self.used_for_boosters) and mappa.valid_and_inside(res):
                self.used_for_boosters.add(res)
                self.used_for_something.add(res)
                return res

    def gen_location(self):
        while True:
            res = self.rand_point()
            if res not in self.used_for_something and self.is_very_good(res):
                self.used_for_boosters.add(res)
                self.used_for_something.add(res)
                return res

    def gen_boosters(self, ch, count, task_spec, mappa):
        for i in range(count):
            task_spec.boosters.add((ch, self.gen_booster_point(mappa)))

    def is_good(self, pt):
        x, y = pt
        state = self.get_state(pt)
        return (state in [State.UNKNOWN, State.GOOD, State.CONTOUR])

    def is_very_good(self, pt):
        x, y = pt
        return (self.get_state(pt) in [State.GOOD])

    def find_lowest_left(self):
        size = self.size
        for y in range(0, size):
            for x in range(0, size):
                if self.get_state((x, y)) in [State.UNKNOWN, State.GOOD, State.CONTOUR]:
                    return (x, y)

    def get_row_req(self, pt, forward, state):
        return [
            (pt, state),
            (next_point(pt, forward), state),
            (prev_point(pt, forward), state)
        ]

    def is_fillable(self, pt, forward):
        right = rotate_clockwise(forward)
        left = rotate_counter_clockwise(forward)
        reqs = (
            self.get_row_req(pt, forward, State.CONTOUR) +
            self.get_row_req(next_point(pt, right), forward, State.FILLED) +
            self.get_row_req(next_point(pt, left), forward, State.UNKNOWN)
        )
        for pt, state in reqs:
            if self.get_state(pt) != state:
                return False
        return True

    def fill_corners(self, num_corners):
        start = self.find_lowest_left()
        forward = (1, 0)
        right = rotate_clockwise(forward)
        left = rotate_counter_clockwise(forward)
        now = start

        while True:
            if self.is_fillable(now, forward):
                # print('filling ', now)
                self.set_state(now, State.FILLED)
                num_corners -= 4
                if num_corners < 0:
                    break

            x, y = now
            left_pt = next_point(now, left)
            forward_pt = next_point(now, forward)
            right_pt = next_point(now, right)

            if self.is_good(right_pt):
                left = forward
                forward = right
                right = rotate_clockwise(right)
                now = right_pt
            elif self.is_good(forward_pt):
                now = forward_pt
            elif self.is_good(left_pt):
                right = forward
                forward = left
                left = rotate_counter_clockwise(left)
                now = left_pt
            else:
                # same as left, but stay on the spot
                right = forward
                forward = left
                left = rotate_counter_clockwise(left)

    def generate_contour(self, debug=False):
        start = self.find_lowest_left()
        # print(start)
        forward = (1, 0)
        right = rotate_clockwise(forward)
        left = rotate_counter_clockwise(forward)
        points = list()
        now = start
        while True:
            last = now == start and len(points) > 0
            if (self.get_state(now) == State.UNKNOWN):
                self.set_state(now, State.CONTOUR)
            x, y = now
            left_pt = next_point(now, left)
            forward_pt = next_point(now, forward)
            right_pt = next_point(now, right)

            if self.is_good(right_pt):
                points.append(next_corner(now, forward, right))
                if debug and len(points) > 1:
                    x1, y1 = points[-2]
                    x2, y2 = points[-1]
                    # print(points[-2], "→", points[-1], "turn right")
                    assert x1 == x2 or y1 == y2
                left = forward
                forward = right
                right = rotate_clockwise(right)
                now = right_pt
            elif self.is_good(forward_pt):
                now = forward_pt
            elif self.is_good(left_pt):
                points.append(next_corner(now, forward, left))
                if debug and len(points) > 1:
                    x1, y1 = points[-2]
                    x2, y2 = points[-1]
                    # print(points[-2], "→", points[-1], "turn left")
                    assert x1 == x2 or y1 == y2
                right = forward
                forward = left
                left = rotate_counter_clockwise(left)
                now = left_pt
            else:
                # same as left, but stay on the spot
                last = False
                points.append(next_corner(now, forward, left))
                if debug and len(points) > 1:
                    x1, y1 = points[-2]
                    x2, y2 = points[-1]
                    # print(points[-2], "→", points[-1], "force left")
                    assert x1 == x2 or y1 == y2
                right = forward
                forward = left
                left = rotate_counter_clockwise(left)

            if last:
                break

        # self.show()
        # print('num turns = ', len(points))
        return points

    def solve(self):
        # create ostov tree for red
        size = self.size

        # virtual border no longer needed
        # for x in range(0, size):
        #     for y in range(0, size):
        #         self.field[x][0] = State.FILLED
        #         self.field[x][size - 1] = State.FILLED
        #         self.field[0][y] = State.FILLED
        #         self.field[size - 1][y] = State.FILLED

        pts = self.get_points_order()
        for pt in pts:
            # print ('bfs from ', pt)
            self.bfs_to_filled(pt)
            # input(">")

        self.show()

        contour = self.generate_contour()
        num_turns = len(contour)
        print("Initial #turns: ", num_turns)

        self.show()

        if num_turns < self.spec.vMin:
            to_fill = self.spec.vMin - num_turns
            # print("filling turns: ", to_fill)
            self.fill_corners(to_fill)
            # self.show()
            contour = self.generate_contour(True)
            # self.show()
            assert len(contour) >= self.spec.vMin

        task_spec = TaskSpec()
        task_spec.contour = contour
        task_spec.boosters = set()

        task_spec.location = self.gen_location()

        self.show()
        print("#points: ", len(contour))
        mappa = Mappa(contour, [], task_spec.location)


        spec = self.spec
        for ch, count in [
            ('B', spec.mNum),
            ('F', spec.fNum),
            ('L', spec.dNum),
            ('R', spec.rNum),
            ('C', spec.cNum),
            ('X', spec.xNum)
        ]:
            self.gen_boosters(ch, count, task_spec, mappa)

        self.show(task_spec)

        self.show_map(mappa, self.spec, task_spec.location)

        return task_spec

    def show_map(self, mappa, spec, location):
        if not self.draw:
            return
        img = Image.new('RGB', (self.size, self.size))

        def pp(x, y, color):
            img.putpixel((x, self.size - 1 - y), color)

        for x in range(0, self.size):
            for y in range(0, self.size):
                pp(x, y, (255, 255, 255) if mappa.inside((x, y)) else (0, 0, 0))

        for p in spec.included:
            pp(p[0], p[1], (0, 255, 0) if mappa.inside(p) else (0, 255, 255))
        for p in spec.excluded:
            pp(p[0], p[1], (255, 0, 0) if not mappa.inside(p) else (255, 255, 0))

        pp(location[0], location[1], (0, 0, 255))

        scale = int(500 / self.size)
        img = img.resize((scale * self.size, scale * self.size))
        img.show()
        img.save(f"images/{self.counter}.png")
        self.counter += 1


    def show(self, task_spec=None):
        if not self.draw:
            return
        img = Image.new('RGB', (self.size, self.size))
        for x in range(0, self.size):
            for y in range(0, self.size):
                state = self.get_state((x, y))
                img.putpixel((x, self.size - 1 - y),
                             get_color(state))

        if task_spec:
            for b in task_spec.boosters:
                img.putpixel((b[1][0], self.size - 1 -
                              b[1][1]), (255, 255, 255))
        # img = img.resize((1000, 1000), Image.BILINEAR)
        scale = int(500 / self.size)
        img = img.resize((scale * self.size, scale * self.size))
        img.show()
        img.save(f"images/{self.counter}.png")
        self.counter += 1
        # input("waiting >")


draw = len(sys.argv) > 3
file = sys.argv[1]
fout = sys.argv[2]
s = read_file(file)

spec = PuzzleSpec(s)

solver = PuzzleSolver(spec, draw)
task_spec = solver.solve()

write_problem(fout, task_spec)
world = parse_problem(read_file(sys.argv[2]))
valid = puzzle_valid(spec, world)
print("valid", valid)
if not valid:
    sys.exit(1)
