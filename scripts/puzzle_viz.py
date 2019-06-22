from PIL import Image

from puzzle_parser import PuzzleSpec
from parser import read_file

import sys

RED = (255, 0, 0)
GREEN = (0, 255, 0)
BLACK = (0, 0, 0)

file = sys.argv[1]
s = read_file(file)

spec = PuzzleSpec(s)
print (f"corners: [{spec.vMin}, {spec.vMax}]")
print (f"num included: {len(spec.included)}")
print (f"num excluded: {len(spec.excluded)}")


img = Image.new('RGB', (spec.tSize, spec.tSize), BLACK)
print (img)

for pt in spec.included:
    img.putpixel(pt, GREEN)

for pt in spec.excluded:
    img.putpixel(pt, RED)

tn_image = img.resize((1024, 1024))

tn_image.show()
tn_image.save('image.png')
