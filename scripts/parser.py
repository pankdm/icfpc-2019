
from world import (
    World,
    Mappa,
    Action,
    Boosters
)

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

    return Boosters(res)

def is_valid_booster(s):
    assert(len(s) == 1)
    return (s in ['B', 'F', 'L', 'R', 'X', 'C'])


def parse_problem(s):
    s = s.strip()
    parts = s.split('#')
    assert(len(parts) == 4)
    contour = parse_map(parts[0])
    assert(contour is not None)
    location = parse_point(parts[1])
    obstacles = parse_obstacles(parts[2])
    boosters = parse_boosters(parts[3])

    mappa = Mappa(contour, obstacles, location)
    world = World(location, mappa, boosters)

    return world


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