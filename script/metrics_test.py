import pandas as pd
import re
import unittest

from metrics import *

columns = 'time group individual group_color individual_color'.split(' ')

class TestAbsenteeism(unittest.TestCase):

    def test_none(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2', 't3', 't3', 't4', 't4'],
                'group'            : ['g1', 'g2', 'g1', 'g2', 'g1', 'g2', 'g1', 'g2'],
                'individual'       : ['i1', 'i2', 'i1', 'i2', 'i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    2,    1,    2,    1,    2,    1,    2],
                'individual_color' : [   1,    2,    1,    2,    1,    2,    1,    2],
            },
            columns = columns,
        )
        got = absenteeism(df)
        self.assertEqual(got, [0, 0], "got -> want")

    def test_always(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2', 't3', 't3', 't4', 't4'],
                'group'            : ['g1', 'g2', 'g1', 'g2', 'g1', 'g2', 'g1', 'g2'],
                'individual'       : ['i1', 'i2', 'i1', 'i2', 'i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    2,    1,    2,    1,    2,    1,    2],
                'individual_color' : [   1,    1,    1,    1,    1,    1,    1,    1],
            },
            columns = columns,
        )
        got = absenteeism(df)
        self.assertEqual(got, [0, 1], "got -> want")

    def test_sometimes(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2', 't3', 't3', 't4', 't4'],
                'group'            : ['g1', 'g1', None, 'g1', 'g1', 'g2', 'g1', 'g1'],
                'individual'       : ['i1', 'i2', 'i1', 'i2', 'i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    1, None,    1,    1,    2,    1,    1],
                'individual_color' : [   1,    1,    1,    1,    2,    2,    1,    1],
            },
            columns = columns,
        )
        got = absenteeism(df)
        self.assertEqual(got, [0.5, 0.0], "got -> want")

class TestInquisitiveness(unittest.TestCase):

    def test_none(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2', 't3', 't3', 't4', 't4'],
                'group'            : ['g1', 'g2', 'g1', 'g2', 'g1', 'g2', 'g1', 'g2'],
                'individual'       : ['i1', 'i2', 'i1', 'i2', 'i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    2,    1,    2,    1,    2,    1,    2],
                'individual_color' : [   1,    2,    1,    2,    1,    2,    1,    2],
            },
            columns = columns,
        )
        got = inquisitiveness(df)
        self.assertEqual(got, [0, 0], "got -> want")

    def test_always(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2', 't3', 't3', 't4', 't4'],
                'group'            : ['g1', 'g2', 'g1', 'g2', 'g1', 'g2', 'g1', 'g2'],
                'individual'       : ['i1', 'i2', 'i1', 'i2', 'i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    2,    1,    2,    1,    2,    1,    2],
                'individual_color' : [   1,    1,    1,    1,    1,    1,    1,    1],
            },
            columns = columns,
        )
        got = inquisitiveness(df)
        self.assertEqual(got, [0, 1], "got -> want")

    def test_absent_is_not_visit(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2', 't3', 't3', 't4', 't4'],
                'group'            : ['g1', 'g1', None, 'g1', 'g1', 'g2', 'g1', 'g1'],
                'individual'       : ['i1', 'i2', 'i1', 'i2', 'i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    1, None,    1,    1,    2,    1,    1],
                'individual_color' : [   1,    1,    1,    1,    2,    2,    1,    1],
            },
            columns = columns,
        )
        got = inquisitiveness(df)
        self.assertEqual(got, [1./3, 0.0], "got -> want")

if __name__ == '__main__':
    unittest.main()
