import unittest

from path import parse_token


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

if __name__ == '__main__':
    unittest.main()
