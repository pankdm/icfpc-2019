
from parser import parse_map


class PuzzleSpec:
    # s is input string data
    def __init__(self, s):
        parts = s.strip('\n').split('#')
        cc = [int(x) for x in parts[0].split(',')]

        self.bNum = cc[0]
        self.eNum = cc[1]
        self.tSize = cc[2]
        self.vMin = cc[3]
        self.vMax = cc[4]
        self.mNum = cc[5]
        self.fNum = cc[6]
        self.dNum = cc[7]
        self.rNum = cc[8]
        self.cNum = cc[9]
        self.xNum = cc[10]

        self.included = parse_map(parts[1])
        self.excluded = parse_map(parts[2])
