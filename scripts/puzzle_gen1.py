import sys

from puzzle_parser import PuzzleSpec
from parser import read_file, parse_problem, write_problem
from pprint import pprint, pformat

from world import (
    World,
    Mappa,
    Action,
    Boosters
)

fIn = sys.argv[1]
fOut = sys.argv[2]
s = read_file(fIn)

spec = PuzzleSpec(s)

world = parse_problem(read_file("problems/chain-puzzle-examples/task.desc"))
print(world.__dict__)
write_problem(fOut, world)
