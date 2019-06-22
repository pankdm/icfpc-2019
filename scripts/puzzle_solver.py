
from PIL import Image

from puzzle_parser import PuzzleSpec
from parser import read_file, write_problem
from puzzle_validator import puzzle_valid

import sys


from copy import deepcopy
from collections import deque
from random import randint

from world import World


class State:
    UNKNOWN = 0
    GOOD = 1
    BAD = 2
    FILLED = 3
    CONNECTED = 4

def get_color(state):
    colors = {
        State.UNKNOWN: (0, 0, 0), # black
        State.GOOD : (0, 255, 0), # green
        State.BAD : (255, 0, 0), # red
        State.FILLED : (255, 255, 0), # yellow
        State.CONNECTED: (255, 0, 0), # red
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

        for pt in spec.included:
            x, y = pt
            self.field[x][y] = State.GOOD

        for pt in spec.excluded:
            x, y = pt
            self.field[x][y] = State.BAD

        self.world = None
        self.used_for_boosters = set()

    def get_points_order(self):
        pts = [pt for pt in spec.excluded]
        pts.sort(key = lambda x : manhattan(x, self.size))
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
                if self.field[x][y] in [State.FILLED, State.CONNECTED]:
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
            x = randint(0, self.spec.tSize)
            y = randint(0, self.spec.tSize)
            res = (x, y)
            if not res in self.used_for_boosters and self.world.inside(res):
                self.used_for_boosters.add(res)
            return res
        
    def gen_boosters(self, ch, count):
        for i in range(count):
            world.boosters.add_booster(ch, self.gen_booster_point())

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

        contour = []

        self.world = World(contour, [], self.gen_booster_point())
        spec = self.spec
        for ch, count in [('M', spec.mNum), ('F', spec.fNum), ('L', spec.dNum), ('R', spec.rNum), ('C', spec.cNum), ('X', spec.xNum)]:
            self.gen_boosters(ch, count)

        self.show()

    def show(self):
        img = Image.new('RGB', (self.size, self.size))
        for x in range(0, self.size):
            for y in range(0, self.size):
                img.putpixel((x, y), get_color(self.field[x][y]))
        img = img.resize((400, 400), Image.BILINEAR)
        img.show()
        img.save('image.png')

file = sys.argv[1]
s = read_file(file)

spec = PuzzleSpec(s)

solver = PuzzleSolver(spec)
solver.solve()

world = solver.world
print(puzzle_valid(spec, world))
write_problem(fOut, world)
