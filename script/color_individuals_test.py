import unittest
from color_individuals import color_individuals
from test_util import expand

class TestIndividualColoring(unittest.TestCase):
    def test_individual_coloring(self):
        subtests = [
            {
                'name' : 'unit costs',
                'input' : {
                    'sw' : 1, 'ab' : 1, 'vi' : 1,
                    'tgi' : [
                      ('t1', 'g1', 'i1'),
                      ('t1', 'g2', 'i2'),
                      ('t2', 'g1', 'i1'),
                      ('t2', 'g2', 'i2'),
                    ],
                    'tg_color' : {
                      't1': { 'g1': 1, 'g2': 2 },
                      't2': { 'g1': 1, 'g2': 2 },
                    },
                },
                'want_cost' : 0,
                'want_it_color' : {
                    ('i1', 't1'): 1,
                    ('i1', 't2'): 1,
                    ('i2', 't1'): 2,
                    ('i2', 't2'): 2,
                },
            },
            {
                'name' : 'cheap vi',
                'input' : {
                    'sw' : 10, 'ab' : 10, 'vi' : 1,
                    'tgi' : {
                        't1': {
                            'g1' : ['i1', 'i2'],
                        },
                        't2': {
                            'g1' : 'i1',
                            'g2' : 'i2',
                        },
                        't3': {
                            'g1' : ['i1', 'i2'],
                        },
                    },
                    'tg_color' : {
                      't1': { 'g1': 1, 'g2': 2 },
                      't2': { 'g1': 1, 'g2': 2 },
                      't3': { 'g1': 1, 'g2': 2 },
                    },
                },
                'want_cost' : 3,
                'want_it_color' : {
                    ('i1', 't3'): 1,
                    ('i1', 't2'): 1,
                    ('i1', 't1'): 1,
                    ('i2', 't3'): 0,
                    ('i2', 't2'): 0,
                    ('i2', 't1'): 0,
                },
            },
            {
                'name' : "cheap ab/vi 1",
                'input' : {
                    'sw': 10, 'ab': 1, 'vi': 2,
                    'tgi': {
                        't1' : {
                            'g1' : [ 'i1', 'i2', 'i3' ],
                        },
                        't2' : {
                            'g1' : [ 'i1', 'i2' ],
                            'g2' : [ 'i3' ],
                        },
                        't3' : {
                            'g1' : [ 'i1', 'i2', 'i3' ],
                        },
                    },
                    'tg_color': {
                        't1': { 'g1': 1, 'g2': 2 },
                        't2': { 'g1': 1, 'g2': 2 },
                        't3': { 'g1': 1, 'g2': 2 },
                    }
                },
                'want_cost' : 3, # ab + vi
                'want_it_color' : {
                    ('i1', 't1') : 1,
                    ('i1', 't2') : 1,
                    ('i1', 't3') : 1,
                    ('i2', 't1') : 1,
                    ('i2', 't2') : 1,
                    ('i2', 't3') : 1,
                    ('i3', 't1') : 1,
                    ('i3', 't2') : 1,
                    ('i3', 't3') : 1,
                }
            },
            {
                'name': "cheap sw",
                'input' : {
                    'sw': 1, 'ab': 10, 'vi': 10,
                    'tgi': [
                        ('t1', 'g1', 'i1'),
                        ('t2', 'g1', 'i1'),
                        ('t3', 'g1', 'i1'),
                    ],
                    'tg_color': {
                        't1': { 'g1': 1 },
                        't2': { 'g1': 2 },
                        't3': { 'g1': 1 },
                    }
                },
                'want_cost' : 2, # sw + sw
                'want_it_color': {
                    ('i1', 't1'): 1,
                    ('i1', 't2'): 2,
                    ('i1', 't3'): 1,
                }
            },
            {
                'name': "cheap ab/vi 2",
                'input' : {
                    'sw': 10, 'ab': 1, 'vi': 1,
                    'tgi': {
                        't1' : {
                            'g1' : [ 'i1' ],
                            'g2' : [],
                            'g3' : [],
                        },
                        't2' : {
                            'g1' : [ 'i1' ],
                            'g2' : [],
                            'g3' : [],
                        },
                        't3' : {
                            'g1' : [ 'i1' ],
                            'g2' : [],
                            'g3' : [],
                        },
                    },
                    'tg_color': {
                        't1': { 'g1': 1, 'g2': 2, 'g3': 3 },
                        't2': { 'g1': 2, 'g2': 3, 'g3': 1 },
                        't3': { 'g1': 3, 'g2': 1, 'g3': 2 },
                    },
                },
                'want_cost' : 3, # vi + ab + ab
                'want_it_color': {
                    ('i1', 't1'): 0,
                    ('i1', 't2'): 0,
                    ('i1', 't3'): 0,
                },
            },
        ]

        for subtest in subtests:
            with self.subTest(subtest['name']):
                test_input = subtest['input']
                test_input['tgi'] = expand(test_input['tgi'])
                cost, it_color = color_individuals(**test_input)
                self.assertEqual(cost, subtest['want_cost'])
                self.assertEqual(it_color, subtest['want_it_color'])

if __name__ == '__main__':
    unittest.main()
