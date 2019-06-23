import sys

import world
import puzzle_parser

def is_good_coordinate(x, spec):
    if x < 0:
        return False
    if x > spec.tSize:
        return False
    return True

def is_good_point(p, spec):
    return is_good_coordinate(p[0], spec) and is_good_coordinate(p[1], spec)

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
    if not world.mappa.valid_and_inside(world.get_location()):
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
        if not world.mappa.valid_and_inside(i):
            print("Missing included (%d, %d)" % (i[0], i[1]))
            return False

    for i in spec.excluded:
        if world.mappa.valid_and_inside(i):
            print("Missing excluded (%d, %d)" % (i[0], i[1]))
            return False

    for b in world.boosters.toList():
        if not is_good_point(b[1], spec):
            print("Wrong booster position (%d, %d) box" % (b[1][0], b[1][1]))
            return False
        if not world.mappa.valid_and_inside(b[1]):
            print("Wrong booster position (%d, %d) inside" % (b[1][0], b[1][1]))
            return False

    bpoints = set()
    added = 1
    bpoints.add(world.get_location())
    for b in world.boosters.toList():
        if b[1] in bpoints:
            print("dup (%d, %d)" % (b[1][0], b[1][1]))
        bpoints.add(b[1])
        added += 1

    if added != len(bpoints):
        print("Boosters intersect %d %d" % (added, len(bpoints)))
        return False

    return True
