import re

FAST_WHEEL_STEPS = 50
DRILL_STEPS = 30

MAP_REGEX = re.compile(r"""((\(\d+,\d+\)),)*(\(\d+,\d+\))""")

class Cell:
    UNKNOWN = 0
    EMPTY = 1
    BLOCK = 2
    WRAPPED = 3

def cell_to_string(color):
    if color == PT_UNKNOWN:
        return "?"
    if color == PT_EMPTY:
        return "."
    if color == PT_BLOCK:
        return "#"
    if color == PT_WRAPPED:
        return " "

class Item:
    MANIPULATOR = 'B'
    WHEEL = 'F'
    DRILL = 'L'
    MYSTERY = 'X'
    TELEPORT = 'R'
    CLONE = 'C'
