

class Map:
    pass


def parse_point(s):
    assert(s[0] == '(')
    assert(s[-1] == ')')
    x, y = map(int, s[1:-1].split(','))
    return (x, y)

def is_valid_booster(s):
    assert(len(s) == 1)
    return (s in ['B', 'F', 'L', 'X'])

def parse_booster(s):
    pass


def parse_boosters(s):
    pass


def parse_problem(s):
    parts = s.split('#')
    m = parse_map(parts[0])
    location = parse_point(parts[1])
    obstacles = parse_obstacles(parts[2])
    boosters = parse_boosters(parts[3])
