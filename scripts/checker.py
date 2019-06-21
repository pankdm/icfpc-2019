#!/usr/bin/env python3


import re

MAP_REGEX = re.compile(r"""((\(\d+,\d+\)),)*(\(\d+,\d+\))""")

PT_UNKNOWN = 0
PT_EMPTY = 1
PT_BLOCK = 2

class Desc:
    def init_box(self, contour):
        # init the bounding box
        self.xmin = self.xmax = contour[0][0]
        self.ymin = self.ymax = contour[0][1]
        for (x, y) in contour:
            self.xmin = min(self.xmin, x)
            self.ymin = min(self.ymin, y)
            self.xmax = max(self.xmax, x)
            self.ymax = max(self.ymax, y)
        self.field = [[PT_UNKNOWN] * self.ymax for i in range(self.xmax)]

    def get_color(self, pt):
        return self.field[pt[0]][pt[1]]

    def set_color(self, pt, color):
        self.field[pt[0]][pt[1]] = color


    # for large maps fails with
    # RecursionError: maximum recursion depth exceeded
    def dfs(self, pt, fill_color, visited):
        if not (0 <= pt[0] < self.xmax and 0 <= pt[1] < self.ymax):
            return
        if self.get_color(pt) not in (PT_UNKNOWN, fill_color):
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
            if self.get_color(pt) not in (PT_UNKNOWN, fill_color):
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




    def __init__(self, contour, obstacles, location, boosters):
        self.loc = location
        self.boosters = boosters
        self.init_box(contour)
        self.draw_contour(contour, PT_EMPTY)
        for ob in obstacles:
            self.draw_contour(ob, PT_BLOCK)

        for x in range(0, self.xmax):
            if self.get_color((x, 0)) == PT_UNKNOWN:
                self.dfs_iter((x,0), PT_BLOCK)
            if self.get_color((x, self.ymax-1)) == PT_UNKNOWN:
                self.dfs_iter((x, self.ymax-1), PT_BLOCK)

        for y in range(0, self.ymax):
            if self.get_color((0, y)) == PT_UNKNOWN:
                self.dfs_iter((0, y), PT_BLOCK)
            if self.get_color((self.xmax-1, y)) == PT_UNKNOWN:
                self.dfs_iter((self.xmax-1, y), PT_BLOCK)

        self.dfs_iter(self.loc, PT_EMPTY)

        for x in range(0, self.xmax):
            for y in range(0, self.ymax):
                if self.get_color((x, y)) == PT_UNKNOWN:
                    self.set_color((x, y), PT_BLOCK)


def parse_point(s):
    assert(s[0] == '(')
    assert(s[-1] == ')')
    x, y = map(int, s[1:-1].split(','))
    return (x, y)

def parse_map(s):
    if len(s)==0:
        return None
    assert (s[0] == '(')
    assert (s[-1] == ')')
    return [tuple(map(int, pair.split(','))) for pair in s[1:-1].split('),(')]

def parse_obstacles(s):
    return [parse_map(obs) for obs in s.split(';') if obs is not None]


def parse_boosters(s):
    res = {}
    for boo in s.split(';'):
        if len(boo) == 0:
            continue
        assert(is_valid_booster(boo[0]))
        pt = parse_point(boo[1:])
        assert(pt not in res)
        res[pt] = boo[0]

    return res

def is_valid_booster(s):
    assert(len(s) == 1)
    return (s in ['B', 'F', 'L', 'R', 'X'])


def parse_problem(s):
    s = s.strip()
    parts = s.split('#')
    assert(len(parts) == 4)
    contour = parse_map(parts[0])
    assert(contour is not None)
    location = parse_point(parts[1])
    obstacles = parse_obstacles(parts[2])
    boosters = parse_boosters(parts[3])
    desc = Desc(contour, obstacles, location, boosters)
    return desc

