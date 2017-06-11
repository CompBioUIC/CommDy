import sys
from cvxopt import solvers, matrix, spmatrix, sparse
from itertools import groupby
from pandas import DataFrame

# Suppress the progress message during solvers.lp().
solvers.options['show_progress'] = False

class UnionFind:
    def __init__(self):
        self.parent = {}
        self.rank = {}

    def MakeSet(self, x):
        if x not in self.parent:
            self.parent[x] = x
            self.rank[x] = 0

    def Find(self, x):
        self.MakeSet(x)
        p = self.parent[x]
        if p != x:
            leader = self.parent[x] = self.Find(p)
        else:
            leader = self.parent[x] = x
        return leader

    def Union(self, x1, x2):
        self.MakeSet(x1)
        self.MakeSet(x2)
        x1 = self.Find(x1)
        x2 = self.Find(x2)
        r1 = self.rank[x1]
        r2 = self.rank[x2]
        if r1 == r2:
            self.rank[x1] += 1
        elif r1 < r2:
            x1, x2 = x2, x1
        self.parent[x2] = x1
        return x1

    def Partition(self):
        members = {}
        for x in self.parent:
            p = self.Find(x)
            if p in members:
                members[p].append(x)
            else:
                members[p] = [x]
        return members.values()

# Creates a linear program from the given data frame.
# The LP has the following form.
#        minimize:         w' * x_e
#        subject to:
#           sum_{e: e = (u, v) in E} x_e <= 1, for each node u.
#           sum_{e: e = (u, v) in E} x_e <= 1, for each node v.
#                                    x_e >= 0, for each edge e.
#
# Note that solvers.lp() accepts an LP of the following form:
#        minimize:         c' * x
#        subject to:       A  * x <= b
# So we multiple -1 through the last set of constraints (x_e >= 0) to get it
# in the accepted form.
def create_linear_program(df, debug=False):
    t_min = df.time.min()
    t_max = df.time.max()
    w = {}

    # Creates nodes and edges.
    node_set = set()
    for i, gb in df.groupby(["individual"]):
        real_nodes = ["%s.%s"%tg for tg in zip(gb.time, gb.group)]
        nodes = []
        if gb.time.iloc[0] != t_min:
            # Create a fake node for a group that appears first in the second
            # time step or later so that the optimal path-cover is not forced
            # to be connected to some small group in a preceeding time step.
            nodes.append("%s.%s.first"%(gb.time.iloc[0], gb.group.iloc[0]))
        nodes.extend(real_nodes)
        if gb.time.iloc[-1] != t_max and t_min != t_max:
            # Create a fake node for a group that appear last in the second to
            # last time step or earlier so that the optimal path-cover is not
            # forced to be connected to some small group in a succeeding time
            # step.
            nodes.append("%s.%s.last"%(gb.time.iloc[-1], gb.group.iloc[-1]))
        # Update the set of all nodes.
        node_set.update(nodes)
        # Add edges between real/fake nodes with weight 1 per individual.
        for edge in zip(nodes, nodes[1:]):
            if edge in w:
                w[edge] += 1.0
            else:
                w[edge] = 1.0
        # Add time-warping edges between real nodes with an epsilon weight to
        # artificially reduce the number of colors used.
        epsilon = 0.01 / len(real_nodes)**2
        for i in range(len(real_nodes)):
            for j in range(i+2, len(real_nodes)):
                edge = (real_nodes[i], real_nodes[j])
                w[edge] = epsilon
    all_nodes = sorted(node_set)
    all_edges = sorted(w)
    n = len(all_nodes)
    m = len(all_edges)
    # Numerize nodes.
    node_idx = {u: i for i, u in enumerate(all_nodes)}

    # Create a constraint n,m-matrix.
    A = sparse([
        # At most one outgoing path.
        spmatrix(1.0, [node_idx[u] for u, _ in all_edges], range(m), (n, m)),
        # At most one incoming path.
        spmatrix(1.0, [node_idx[u] for _, u in all_edges], range(m), (n, m)),
        # Non-negative constraints.
        spmatrix(-1.0, range(m), range(m)),
    ])
    b = matrix([1.0] * n * 2 + [0.0] * m)
    #print("A b:", sparse([[A], [b]]))
    # objective function.
    c = matrix([float(-w[e]) for e in all_edges])
    if debug:
        print("c:", c)
    return c, A, b, all_nodes, all_edges

def convert_lp_solution_to_coloring(edges, sol, debug=False):
    X = sol['x']
    if debug:
        print("X:", X)
        print("edges:", edges)
    uf = UnionFind()
    timestamp_cover = {}
    for x, (n1, n2) in sorted(zip(X, edges), reverse=True):
        if not n1.endswith('.first'):
            uf.MakeSet(n1)
        if not n2.endswith('.last'):
            uf.MakeSet(n2)
        if x < 1e-5:
            continue
        if n1.endswith('.first') or n2.endswith('.last'):
            continue
        t1, g1 = n1.split('.')
        t2, g2 = n2.split('.')
        p1 = uf.Find(n1)
        p2 = uf.Find(n2)
        if p1 in timestamp_cover:
            cover1 = timestamp_cover[p1]
        else:
            cover1 = timestamp_cover[p1] = set([t1])
        if p2 in timestamp_cover:
            cover2 = timestamp_cover[p2]
        else:
            cover2 = timestamp_cover[p2] = set([t2])
        if len(cover1.intersection(cover2)) > 0:
            if debug:
                print("ignore", (t1, g1), (t2, g2), cover1, cover2)
            continue
        if debug:
            print("union", (t1, g1), (t2, g2), cover1, cover2)
        p = uf.Union(n1, n2)
        timestamp_cover[p].update([t1, t2])
    # Assign colors to groups.
    tg_color = {}
    for color_idx, members in enumerate(sorted(uf.Partition(), key=lambda x: (len(x), x), reverse=True)):
        for tg in members:
            t, g = tg.split('.')
            if t not in tg_color:
                tg_color[t] = {}
            tg_color[t][g] = color_idx + 1
    return tg_color

def color_groups(df):
    c, A, b, nodes, edges = create_linear_program(df)
    sol = solvers.lp(c, A, b)
    if sol['status'] != 'optimal':
        print("Solver error:", sol, file=sys.stderr)
        return None
    tg_color = convert_lp_solution_to_coloring(edges, sol)
    return tg_color
