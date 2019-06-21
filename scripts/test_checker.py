import unittest

from checker import parse_point

class TestParse(unittest.TestCase):
    def test_parse_point(self):
        self.assertEqual(parse_point("(1, 1)"), (1, 1))


if __name__ == '__main__':
    unittest.main()
