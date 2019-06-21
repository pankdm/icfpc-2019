

class World:
    def __init__(self):
        pass

class Action:
    def __init__(self):
        self.type = None
        self.pt = None

# returns pair of next index, result
def parse_token(path, index):
    assert index < len(path)
    single = set(['W', 'S', 'A', 'D', 'Z', 'E', 'Q', 'F', 'L'])
    with_pt = set(['B', 'T'])
    if path[index] in single:
        action = Action()
        action.type = path[index]
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
        action = Action()
        action.type = path[index]
        action.pt = (x, y)
        return action, index2 + 1

    assert 'Unknown command: ', path[index]
