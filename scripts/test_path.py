import unittest

from path import (
    parse_token,
    parse_path,
    Action,
    Arm
)


class TestPath(unittest.TestCase):
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
        result = parse_path('SB(-1,0)WA')
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


if __name__ == '__main__':
    unittest.main()
