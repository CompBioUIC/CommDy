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

class TestCommunityStay(unittest.TestCase):

    def test_always_stay(self):
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
        got = community_stay(df)
        self.assertEqual(got, [4, 4], "got -> want")

    def test_not_always_stay(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't2', 't3', 't4', 't1', 't2', 't3', 't4'],
                'group'            : ['g1', 'g1', 'g1', 'g1', 'g2', 'g2', 'g2', 'g2'],
                'individual'       : ['i1', 'i1', 'i1', 'i1', 'i2', 'i2', 'i2', 'i2'],
                'group_color'      : [   1,    1,    1,    1,    1,    1,    1,    1],
                'individual_color' : [   1,    1,    2,    2,    1,    2,    1,    2],
            },
            columns = columns,
        )
        got = community_stay(df)
        self.assertEqual(got, [2.0, 1.0], "got -> want")

    def test_with_missing(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2', 't3', 't3', 't4', 't4'],
                'group'            : ['g1', 'g1', None, 'g1', 'g1', 'g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i2', 'i1', 'i2', 'i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    1, None,    1,    1,    1,    1,    1],
                'individual_color' : [   1,    1,    1,    1,    1,    1,    1,    1],
            },
            columns = columns,
        )
        got = community_stay(df)
        self.assertEqual(got, [4, 4], "got -> want")

class TestComputePeers(unittest.TestCase):

    def test_one_group_one_community(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2'],
                'group'            : ['g1', 'g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    1,    1,    1],
                'individual_color' : [   1,    1,    1,    1],
            },
            columns = columns,
        )
        got = compute_subgroups(df)
        want = {
            ('t1', 'g1', 1): set(['i1', 'i2']),
            ('t2', 'g1', 1): set(['i1', 'i2']),
        }
        self.assertEqual(got, want, "got -> want")

    def test_one_group_two_communities(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2'],
                'group'            : ['g1', 'g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    1,    1,    1],
                'individual_color' : [   1,    2,    1,    2],
            },
            columns = columns,
        )
        got = compute_subgroups(df)
        want = {
            ('t1', 'g1', 1): set(['i1']),
            ('t1', 'g1', 2): set(['i2']),
            ('t2', 'g1', 1): set(['i1']),
            ('t2', 'g1', 2): set(['i2']),
        }
        self.assertEqual(got, want, "got -> want")

    def test_ignore_unobserved(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't3'],
                'group'            : ['g1', None, None, 'g1'],
                'individual'       : ['i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1, None, None,    2],
                'individual_color' : [   1,    1,    1,    2],
            },
            columns = columns,
        )
        got = compute_subgroups(df)
        want = {
            ('t1', 'g1', 1): set(['i1']),
            ('t3', 'g1', 2): set(['i2']),
        }
        self.assertEqual(got, want, "got -> want")

class TestAvgNumPeers(unittest.TestCase):

    def test_one_group_one_community(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2', 't2'],
                'group'            : ['g1', 'g1', 'g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i2', 'i1', 'i2', 'i3'],
                'group_color'      : [   1,    1,    1,    1,    1],
                'individual_color' : [   1,    1,    1,    1,    1],
            },
            columns = columns,
        )
        tgc_members = compute_subgroups(df)
        got = avg_num_peers(df, tgc_members)
        want = [1.5, 1.5, 2.0]
        self.assertEqual(got, want, "got -> want")

    def test_one_group_two_communities(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't1', 't1'],
                'group'            : ['g1', 'g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i2', 'i3', 'i4'],
                'group_color'      : [   1,    1,    1,    1],
                'individual_color' : [   1,    2,    2,    2],
            },
            columns = columns,
        )
        tgc_members = compute_subgroups(df)
        got = avg_num_peers(df, tgc_members)
        want = [0.0, 2.0, 2.0, 2.0]
        self.assertEqual(got, want, "got -> want")

    def test_two_group_one_community(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't1', 't1'],
                'group'            : ['g1', 'g1', 'g2', 'g2'],
                'individual'       : ['i1', 'i2', 'i3', 'i4'],
                'group_color'      : [   1,    1,    2,    2],
                'individual_color' : [   1,    1,    1,    1],
            },
            columns = columns,
        )
        tgc_members = compute_subgroups(df)
        got = avg_num_peers(df, tgc_members)
        want = [1.0, 1.0, 1.0, 1.0]
        self.assertEqual(got, want, "got -> want")

    def test_ignore_missing(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't1', 't1'],
                'group'            : ['g1', 'g1', 'g1', None],
                'individual'       : ['i1', 'i2', 'i3', 'i4'],
                'group_color'      : [   1,    1,    1, None],
                'individual_color' : [   1,    2,    2,    2],
            },
            columns = columns,
        )
        tgc_members = compute_subgroups(df)
        got = avg_num_peers(df, tgc_members)
        want = [0.0, 1.0, 1.0, 0.0]
        self.assertEqual(got, want, "got -> want")


class TestPeerSynchrony(unittest.TestCase):

    def test_perfect(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2', 't3', 't3'],
                'group'            : ['g1', 'g1', 'g1', 'g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i2', 'i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    1,    1,    1,    1,    1],
                'individual_color' : [   1,    1,    1,    1,    1,    1],
            },
            columns = columns,
        )
        tgc_members = compute_subgroups(df)
        got = peer_synchrony(df, tgc_members)
        want = [1.0, 1.0]
        self.assertEqual(got, want, "got -> want")

    def test_merge(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't1', 't1', 't2', 't2', 't2', 't2'],
                'group'            : ['g1', 'g1', 'g2', 'g2', 'g1', 'g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i2', 'i3', 'i4', 'i1', 'i2', 'i3', 'i4'],
                'group_color'      : [   1,    1,    2,    2,    1,    1,    1,    1],
                'individual_color' : [   1,    1,    1,    1,    1,    1,    1,    1],
            },
            columns = columns,
        )
        tgc_members = compute_subgroups(df)
        got = peer_synchrony(df, tgc_members)
        want = [1./3] * 4
        self.assertEqual(got, want, "got -> want")

    def test_split(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't1', 't1', 't2', 't2', 't2', 't2'],
                'group'            : ['g1', 'g1', 'g1', 'g1', 'g1', 'g1', 'g2', 'g2'],
                'individual'       : ['i1', 'i2', 'i3', 'i4', 'i1', 'i2', 'i3', 'i4'],
                'group_color'      : [   1,    1,    1,    1,    1,    1,    2,    2],
                'individual_color' : [   1,    1,    1,    1,    1,    1,    1,    1],
            },
            columns = columns,
        )
        tgc_members = compute_subgroups(df)
        got = peer_synchrony(df, tgc_members)
        want = [1.0, 1.0, 1.0, 1.0]
        self.assertEqual(got, want, "got -> want")

    def test_include_missing(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't1', 't2', 't2', 't2', 't3', 't3', 't3'],
                'group'            : ['g1', 'g1', 'g1', 'g1', 'g1', None, 'g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i2', 'i3', 'i1', 'i2', 'i3', 'i1', 'i2', 'i3'],
                'group_color'      : [   1,    1,    1,    1,    1, None,    1,    1,    1],
                'individual_color' : [   1,    1,    1,    1,    1,    1,    1,    1,    1],
            },
            columns = columns,
        )
        tgc_members = compute_subgroups(df)
        got = peer_synchrony(df, tgc_members)
        want = [0.75, 0.75, 0.0]
        self.assertEqual(got, want, "got -> want")

class TestGroupSize(unittest.TestCase):

    def test_constant(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2', 't3', 't3'],
                'group'            : ['g1', 'g1', 'g1', 'g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i2', 'i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    1,    1,    1,    1,    1],
                'individual_color' : [   1,    1,    1,    1,    1,    1],
            },
            columns = columns,
        )
        got = group_size(df)
        want = [2.0, 2.0]
        self.assertEqual(got, want, "got -> want")

    def test_sizes_1_2_3(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't2', 't2', 't3', 't3', 't3'],
                'group'            : ['g1', 'g1', 'g1', 'g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i1', 'i2', 'i1', 'i2', 'i3'],
                'group_color'      : [   1,    1,    1,    1,    1,    1],
                'individual_color' : [   1,    1,    1,    1,    1,    1],
            },
            columns = columns,
        )
        got = group_size(df)
        want = [2.0, 2.5, 3.0]
        self.assertEqual(got, want, "got -> want")

    def test_ignore_missing(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2', 't3', 't3'],
                'group'            : [None, 'g1', None, 'g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i2', 'i1', 'i2', 'i1', 'i2'],
                'group_color'      : [None,    1, None,    1,    1,    1],
                'individual_color' : [   1,    1,    1,    1,    1,    1],
            },
            columns = columns,
        )
        got = group_size(df)
        want = [2.0, 4./3]
        self.assertEqual(got, want, "got -> want")

class TestGroupHomogeneity(unittest.TestCase):

    def test_one_zero(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2'],
                'group'            : ['g1', 'g2', 'g1', 'g2'],
                'individual'       : ['i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    1,    1,    1],
                'individual_color' : [   1,    2,    1,    2],
            },
            columns = columns,
        )
        got = group_homogeneity(df)
        want = [1.0, 0.0]
        self.assertEqual(got, want, "got -> want")

    def test_half(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2'],
                'group'            : ['g1', 'g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    1,    1,    1],
                'individual_color' : [   1,    2,    1,    2],
            },
            columns = columns,
        )
        got = group_homogeneity(df)
        want = [0.5, 0.5]
        self.assertEqual(got, want, "got -> want")

    def test_ignore_missing(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't1', 't2', 't2', 't2'],
                'group'            : ['g1', 'g1', None, 'g1', 'g1', None],
                'individual'       : ['i1', 'i2', 'i3', 'i1', 'i2', 'i3'],
                'group_color'      : [   1,    1, None,    1,    1, None],
                'individual_color' : [   1,    1,    1,    1,    1,    1],
            },
            columns = columns,
        )
        got = group_homogeneity(df)
        want = [1.0, 1.0, 0.0]
        self.assertEqual(got, want, "got -> want")

class TestIndividualApparancy(unittest.TestCase):

    def test_two_time_steps(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2'],
                'group'            : ['g1', 'g2', 'g1', 'g2'],
                'individual'       : ['i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    1,    1,    1],
                'individual_color' : [   1,    2,    1,    2],
            },
            columns = columns,
        )
        got = individual_apparency(df)
        want = [2.0, 2.0]
        self.assertEqual(got, want, "got -> want")

    def test_3_time_steps(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't2', 't3'],
                'group'            : ['g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i1', 'i1'],
                'group_color'      : [   1,    1,    2],
                'individual_color' : [   1,    1,    2],
            },
            columns = columns,
        )
        got = individual_apparency(df)
        want = [1.5]
        self.assertEqual(got, want, "got -> want")

    def test_flipping(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't2', 't3'],
                'group'            : ['g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i1', 'i1'],
                'group_color'      : [   1,    2,    1],
                'individual_color' : [   1,    2,    1],
            },
            columns = columns,
        )
        got = individual_apparency(df)
        want = [2.0]
        self.assertEqual(got, want, "got -> want")

class TestCyclicity(unittest.TestCase):

    def test_zero(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2', 't3', 't3'],
                'group'            : ['g1', 'g1', 'g1', 'g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i2', 'i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    1,    1,    1,    1,    1],
                'individual_color' : [   1,    1,    2,    2,    2,    2],
            },
            columns = columns,
        )
        got = cyclicity(df)
        want = [0.0, 0.0]
        self.assertEqual(got, want, "got -> want")

    def test_one(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't2', 't3', 't4', 't1', 't2', 't3', 't4'],
                'group'            : ['g1', 'g1', 'g1', 'g1', 'g1', 'g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i1', 'i1', 'i1', 'i2', 'i2', 'i2', 'i2'],
                'group_color'      : [   1,    1,    1,    1,    1,    1,    1,    1],
                'individual_color' : [   1,    2,    2,    1,    1,    2,    2,    1],
            },
            columns = columns,
        )
        got = cyclicity(df)
        want = [1.0, 1.0]
        self.assertEqual(got, want, "got -> want")

    def test_two(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't2', 't3', 't4', 't5', 't6'],
                'group'            : ['g1', 'g1', 'g1', 'g1', 'g1', 'g1'],
                'individual'       : ['i1', 'i1', 'i1', 'i1', 'i1', 'i1'],
                'group_color'      : [   1,    1,    1,    1,    1,    1],
                'individual_color' : [   1,    2,    2,    1,    2,    2],
            },
            columns = columns,
        )
        got = cyclicity(df)
        want = [2.0]
        self.assertEqual(got, want, "got -> want")

class TestCommunitySize(unittest.TestCase):

    def test_size_two(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't1', 't2', 't2', 't2'],
                'group'            : ['g1', 'g1', 'g2', 'g1', 'g1', 'g2'],
                'individual'       : ['i1', 'i2', 'i3', 'i1', 'i2', 'i3'],
                'group_color'      : [   1,    1,    2,    1,    1,    2],
                'individual_color' : [   1,    1,    2,    1,    1,    2],
            },
            columns = columns,
        )
        got = community_size(df)
        want = [2.0, 2.0, 1.0]
        self.assertEqual(got, want, "got -> want")

    def test_missing_is_included(self):
        df = pd.DataFrame(
            data={
                'time'             : ['t1', 't1', 't2', 't2'],
                'group'            : ['g1', 'g1', 'g1', None],
                'individual'       : ['i1', 'i2', 'i1', 'i2'],
                'group_color'      : [   1,    1,    1, None],
                'individual_color' : [   1,    1,    1,    0],
            },
            columns = columns,
        )
        got = community_size(df)
        want = [1.5, 1.5]
        self.assertEqual(got, want, "got -> want")

if __name__ == '__main__':
    unittest.main()
