import unittest

from checker import parse_point, parse_problem

class TestParse(unittest.TestCase):
    def test_parse_point(self):
        self.assertEqual(parse_point("(1,1)"), (1, 1))

    def test_file(self):
        path = "../problems/part-1-initial/prob-111.desc"
        with open(path) as f:
            s = f.read()
            return parse_problem(s)


if __name__ == '__main__':
    unittest.main()
