import unittest

from cvxopt import solvers
from pathcover import color_groups, color_dataframe
from pandas import DataFrame, read_csv
from pandas.util.testing import assert_frame_equal
from collections import namedtuple
from test_util import expand

def to_dataframe(tgi):
    return DataFrame(data={
        'time': [t for t, _, _ in tgi],
        'group': [g for _, g, _ in tgi],
        'individual': [i for _, _, i in tgi],
    })

TestCase = namedtuple('TestCase', 'name tgi want_eq_classes')

class TestPathCover(unittest.TestCase):

    def test1(self):
        subtests = [
            TestCase(
                name = 'test1',
                tgi = [
                    ("t1", "g1", "i1"),
                    ("t2", "g1", "i1"),
                ],
                want_eq_classes = [
                    [
                        ('t1', 'g1'),
                        ('t2', 'g1'),
                    ],
                ],
            ),
            TestCase(
                name = 'test2',
                tgi = [
                    ('t1', [
                        ('g1', ['i1', 'i2', 'i3']),
                        ('g4', 'i4'),
                    ]),
                    ('t2', [
                        ('g1', ['i1', 'i2']),
                        ('g2', 'i3'),
                        ('g4', 'i4'),
                    ]),
                    ('t3', [
                        ('g1', ['i1', 'i2', 'i3']),
                        ('g4', 'i4'),
                    ]),
                ],
                want_eq_classes = [
                    [
                        ('t1', 'g1'),
                        ('t2', 'g1'),
                        ('t3', 'g1'),
                    ],
                    [
                        ('t1', 'g4'),
                        ('t2', 'g4'),
                        ('t3', 'g4'),
                    ],
                    [
                        ('t2', 'g2'),
                    ],
                ],
            ),
            TestCase(
                name = 'artificial_edges',
                tgi = [
                    ('t1', [
                        ('g1', 'i1'),
                    ]),
                    ('t2', [
                        ('g1', ['i1', 'i2', 'i3', 'i4', 'i5']),
                    ]),
                    ('t3', [
                        ('g1', ['i1', 'i2', 'i3', 'i4', 'i5']),
                    ]),
                    ('t4', [
                        ('g1', 'i1'),
                    ]),
                ],
                want_eq_classes = [
                    [
                        ('t1', 'g1'),
                        ('t4', 'g1'),
                    ],
                    [
                        ('t2', 'g1'),
                        ('t3', 'g1'),
                    ],
                ],
            ),
        ]
        for subtest in subtests:
            with self.subTest(subtest.name):
                df = to_dataframe(expand(subtest.tgi))
                tg_color = color_groups(df)

                color_classes = {}
                for t, gcs in tg_color.items():
                    for g, c in gcs.items():
                        if c in color_classes:
                            color_classes[c].append((t, g))
                        else:
                            color_classes[c] = [(t, g)]
                color_classes = sorted(sorted(cs) for cs in color_classes.values())
                self.assertEqual(len(color_classes), len(subtest.want_eq_classes))
                for i, (got, want) in enumerate(zip(color_classes, subtest.want_eq_classes)):
                    self.assertEqual(sorted(got), want, "i=%d"%i)

    def test_file_111(self):
        df = read_csv('test.csv')
        color_dataframe(df, sw=1, ab=1, vi=1)
        want = read_csv('test_111_want.csv')
        assert_frame_equal(df, want)

    def test_file_119(self):
        df = read_csv('test.csv')
        color_dataframe(df, sw=1, ab=1, vi=9)
        want = read_csv('test_119_want.csv')
        assert_frame_equal(df, want)

    def test2_file_111(self):
        df = read_csv('test2.csv')
        color_dataframe(df, sw=1, ab=1, vi=1)
        want = read_csv('test2_111_want.csv')
        assert_frame_equal(df, want)

    def test2_file_191(self):
        df = read_csv('test2.csv')
        color_dataframe(df, sw=1, ab=9, vi=1)
        want = read_csv('test2_191_want.csv')
        assert_frame_equal(df, want)

if __name__ == '__main__':
    unittest.main()
