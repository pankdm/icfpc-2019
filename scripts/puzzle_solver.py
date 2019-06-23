#!/usr/bin/env python3

from PIL import Image

from puzzle_parser import PuzzleSpec
from parser import read_file, write_problem, TaskSpec
from puzzle_validator import puzzle_valid

import sys


from copy import deepcopy
from collections import deque
from random import randint

from world import World


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
        State.CONTOUR: (0, 255, 0),  # grey
    }
    return colors[state]


def manhattan(pt, size):
    x, y = pt
    return min([x, y, size - 1 - x, size - 1 - y])


class PuzzleSolver:
    def __init__(self, spec):
        self.spec = spec
        self.size = spec.tSize
        self.field = [[State.UNKNOWN] * self.size for x in range(self.size)]

        print(f"min, max: {spec.vMin}, {spec.vMax}")

        for pt in spec.included:
            x, y = pt
            # we don't support greens on border yet
            assert x != 0
            assert x != self.size - 1
            assert y != 0
            assert y != self.size - 1
            self.field[x][y] = State.GOOD

        for pt in spec.excluded:
            x, y = pt
            self.field[x][y] = State.BAD

        self.world = None
        self.used_for_boosters = set()
        self.used_for_something = set()

    def get_points_order(self):
        pts = [pt for pt in spec.excluded]
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
                if self.field[x][y] in [State.FILLED]:
                    target = now
                    break

                if self.field[x][y] == State.UNKNOWN:
                    if next not in dist:
                        dist[next] = d + 1
                        back[next] = now
                        q.append(next)

            if target:
                break

        # do backwards pass
        while target != start:
            x, y = target
            assert self.field[x][y] == State.UNKNOWN
            self.field[x][y] = State.FILLED
            target = back.get(target, None)

        self.set_state(start, State.CONNECTED)

    def set_state(self, pt, state):
        x, y = pt
        self.field[x][y] = state

    def gen_booster_point(self):
        while True:
            x = randint(0, self.spec.tSize - 1)
            y = randint(0, self.spec.tSize - 1)
            res = (x, y)
            if not res in self.used_for_boosters and self.is_good(res):
                self.used_for_boosters.add(res)
                self.used_for_something.add(res)
            return res

    def gen_location(self):
        while True:
            x = randint(0, self.spec.tSize - 1)
            y = randint(0, self.spec.tSize - 1)
            res = (x, y)
            if not res in self.used_for_something and self.is_good(res):
                self.used_for_something.add(res)
            return res

    def gen_boosters(self, ch, count, task_spec):
        for i in range(count):
            task_spec.boosters.add((ch, self.gen_booster_point()))

    def is_good(self, pt):
        x, y = pt
        return (self.field[x][y] in [State.UNKNOWN, State.GOOD, State.CONTOUR])

    def find_lowest_left(self):
        size = self.size
        for x in range(0, size):
            for y in range(0, size):
                if self.field[x][y] in [State.UNKNOWN, State.GOOD]:
                    return (x, y)

    def generate_contour(self):
        start = self.find_lowest_left()
        forward = (1, 0)
        points = list()
        points.append(start)
        now = start
        num_turns = 0
        while True:
            if now == start and forward == (1, 0) and num_turns > 0:
                break
            self.set_state(now, State.CONTOUR)
            x, y = now
            right = rotate_clockwise(forward)
            right_pt = next_point(now, right)
            if self.is_good(right_pt):
                num_turns += 1
                points.append(now)
                forward = right
                now = right_pt

                continue

            forward_pt = next_point(now, forward)
            if self.is_good(forward_pt):
                now = forward_pt
                continue

            left = rotate_counter_clockwise(forward)
            num_turns += 1
            points.append(forward_pt)
            forward = left

        print ('num turns = ', num_turns)
        print (points[-5:])
        return points

    def solve(self):
        # create ostov tree for red
        size = self.size
        for x in range(0, size):
            for y in range(0, size):
                self.field[x][0] = State.FILLED
                self.field[x][size - 1] = State.FILLED
                self.field[0][y] = State.FILLED
                self.field[size - 1][y] = State.FILLED

        pts = self.get_points_order()
        for pt in pts:
            # print ('bfs from ', pt)
            self.bfs_to_filled(pt)
            # input(">")

        task_spec = TaskSpec()
        task_spec.contour = self.generate_contour()
        task_spec.boosters = set()
        task_spec.location = self.gen_location()

        spec = self.spec
        for ch, count in [
            ('M', spec.mNum),
            ('F', spec.fNum),
            ('L', spec.dNum),
            ('R', spec.rNum),
            ('C', spec.cNum),
            ('X', spec.xNum)
        ]:
            self.gen_boosters(ch, count, task_spec)
        return task_spec

        # self.show()

    def show(self):
        img = Image.new('RGB', (self.size, self.size))
        for x in range(0, self.size):
            for y in range(0, self.size):
                img.putpixel((x, self.size - 1 - y),
                             get_color(self.field[x][y]))
        img = img.resize((600, 600), Image.BILINEAR)
        img.show()
        img.save('image.png')


file = sys.argv[1]
fout = sys.argv[2]
s = read_file(file)

spec = PuzzleSpec(s)

solver = PuzzleSolver(spec)
task_spec = solver.solve()

# print(puzzle_valid(spec, world))
write_problem(fout, task_spec)
