import unittest

from parser import (
    parse_token,
    parse_solution,
    parse_point,
    parse_problem,
)

from world import (
    Action,
    Arm,
)

from api import (
    verify_solution_file
)


class TestParse(unittest.TestCase):
    def test_parse_problem(self):
        path = "../problems/part-1-initial/prob-111.desc"
        with open(path) as f:
            s = f.read()
            return parse_problem(s)

    def test_parse_point(self):
        self.assertEqual(parse_point("(1,1)"), (1, 1))

    def test_parse_simple_token(self):
        action, index = parse_token('WASD', 0)
        self.assertEqual(action.type, 'W')
        self.assertEqual(index, 1)

    def test_parse_pt_token(self):
        path = 'SB(-1,0)WA'
        action, index = parse_token(path, 1)
        self.assertEqual(action.type, 'B')
        self.assertEqual(action.pt, (-1, 0))
        self.assertEqual(path[index:], 'WA')

    def test_parse_path(self):
        result = parse_solution('SB(-1,0)WA\n')
        self.assertEqual(len(result), 4)
        self.assertActionEqual(result[0], Action('S'))
        self.assertActionEqual(result[1], Action('B', (-1, 0)))
        self.assertActionEqual(result[2], Action('W'))
        self.assertActionEqual(result[3], Action('A'))

    def test_parse_invalid_token(self):
        with self.assertRaises(AssertionError):
            result = parse_token("PPP", 0)

    def test_rotate(self):
        arm = Arm(1, 0)

        arm.rotate_clockwise()
        self.assertEqual(arm, Arm(0, -1))

        arm.rotate_clockwise()
        self.assertEqual(arm, Arm(-1, 0))

        arm.rotate_clockwise()
        self.assertEqual(arm, Arm(0, 1))

    def test_rotate_counter(self):
        arm = Arm(1, 0)

        arm.rotate_counter_clockwise()
        self.assertEqual(arm, Arm(0, 1))

        arm.rotate_counter_clockwise()
        self.assertEqual(arm, Arm(-1, 0))

        arm.rotate_counter_clockwise()
        self.assertEqual(arm, Arm(0, -1))

    def assertActionEqual(self, a, b):
        self.assertEqual(a.type, b.type)
        self.assertEqual(a.pt, b.pt)


PART1_DIRECTORY = "../problems/part-1-examples/"
PART2_DIRECTORY = "../problems/part-2-teleports-examples/"
PART3_DIRECTORY = "../problems/part-3-clones-examples/"


class TestSolutionScore(unittest.TestCase):
    def test_example1_1(self):
        problem = PART1_DIRECTORY + 'example-01.desc'
        solution1 = PART1_DIRECTORY + 'example-01-1.sol'
        assert verify_solution_file(problem, solution1)

    def test_example1_2(self):
        problem = PART1_DIRECTORY + 'example-01.desc'
        solution2 = PART1_DIRECTORY + 'example-01-2.sol'
        assert verify_solution_file(problem, solution2)

    def test_example1_3(self):
        problem = PART1_DIRECTORY + 'example-01.desc'
        solution3 = PART1_DIRECTORY + 'example-01-3.sol'
        assert verify_solution_file(problem, solution3)


if __name__ == '__main__':
    unittest.main()
