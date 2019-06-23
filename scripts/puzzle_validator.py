import sys

import world
import puzzle_parser

def is_good_coordinate(x, spec):
    if x < 0:
        return False
    if x > spec.tSize:
        return False
    return True

def too_small(p, spec):
    xmin, xmax = p
    return (xmax - xmin + 1) <= spec.tSize - (spec.tSize // 10)

def puzzle_valid(spec, world):
    if not world.mappa:
        print("No mappa")
        return False
    if not world.mappa.contour:
        print("No contour")
        return False
    contour = world.mappa.contour
    for c in contour:
        if not is_good_coordinate(c[0], spec) or not is_good_coordinate(c[1], spec):
            print("Not valid coordinate")
            return False
    if not world.mappa.inside(world.get_location()):
        print("Not inside %s" % (str(world.mappa.get_color(world.get_location()))))
        return False

    box = world.mappa.get_box()
    if too_small(box[0], spec) and too_small(box[1], spec):
        print("Box too small")
        return False

    if world.mappa.area() < (spec.tSize ** 2) // 5:
        print("Area too small")
        return False

    if len(contour) < spec.vMin:
        print("Not enough vertices")
        return False

    if len(contour) > spec.vMax:
        print("Too much vertices")
        return False

    for i in spec.included:
        if not world.mappa.inside(i):
            print("Missing included")
            return False

    for i in spec.excluded:
        if world.mappa.inside(i):
            print("Missing excluded")
            return False

    for b in world.boosters.toList():
        if not world.mappa.inside(b[1]):
            print("Wrong booster position")
            return False

    return True
