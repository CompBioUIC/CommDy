import sys
from cvxopt import solvers, matrix, spmatrix, sparse
from itertools import groupby
from pandas import DataFrame
from color_individuals import color_individuals

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

NODE_TYPE_FIRST = 0
NODE_TYPE_REAL  = 1
NODE_TYPE_LAST  = 2

NODE_TYPE_NAME_SUFFIXES = [".first", "", ".last"]

class Node:
    def __init__(self, t, g, node_type=NODE_TYPE_REAL):
        self.t, self.g, self.node_type = t, g, node_type

    def is_real(self):
        return self.node_type == NODE_TYPE_REAL

    def __repr__(self):
        return "%s.%s%s"%(self.t, self.g, NODE_TYPE_NAME_SUFFIXES[self.node_type])

    # Needed for sort().
    def __lt__(self, other):
        if self.t != other.t:
            return self.t < other.t
        if self.g != other.g:
            return self.g < other.g
        if self.node_type != self.node_type:
            return self.node_type < self.node_type
        return False

    # Needed as a dict key.
    def __hash__(self):
        return hash((self.t, self.g, self.node_type))

    # Needed as a dict key.
    def __eq__(self, other):
        return (self.t, self.g, self.node_type) == (other.t, other.g, other.node_type)

    # Needed as a dict key.
    def __ne__(self, other):
        return not(self == other)

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
def create_linear_program(df, debug=False, reduce_colors=True,
        t_col='time', g_col='group', i_col='individual'):
    t_min = df[t_col].min()
    t_max = df[t_col].max()
    w = {}

    # Creates nodes and edges.
    node_set = set()
    for i, gb in df.groupby(i_col):
        real_nodes = [Node(t, g) for t, g in zip(gb[t_col], gb[g_col])]
        nodes = []
        if gb[t_col].iloc[0] != t_min:
            # Create a fake node for a group that appears first in the second
            # time step or later so that the optimal path-cover is not forced
            # to be connected to some small group in a preceeding time step.
            nodes.append(Node(gb[t_col].iloc[0], gb[g_col].iloc[0], NODE_TYPE_FIRST))
        nodes.extend(real_nodes)
        if gb[t_col].iloc[-1] != t_max and t_min != t_max:
            # Create a fake node for a group that appear last in the second to
            # last time step or earlier so that the optimal path-cover is not
            # forced to be connected to some small group in a succeeding time
            # step.
            nodes.append(Node(gb[t_col].iloc[-1], gb[g_col].iloc[-1], NODE_TYPE_LAST))
        # Update the set of all nodes.
        node_set.update(nodes)
        # Add edges between real/fake nodes with weight 1 per individual.
        for edge in zip(nodes, nodes[1:]):
            if edge in w:
                w[edge] += 1.0
            else:
                w[edge] = 1.0
        if reduce_colors:
            # Add time-warping edges between real nodes with an epsilon weight to
            # artificially reduce the number of colors used.
            epsilon = 0.01 / len(real_nodes)**2
            for i in range(len(real_nodes)):
                for j in range(i+2, len(real_nodes)):
                    edge = (real_nodes[i], real_nodes[j])
                    if not edge in w:
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

def convert_lp_solution_to_coloring(edges, sol, times, debug=False):
    X = sol['x']
    if debug:
        print("X:", X)
        print("edges:", edges)
    uf = UnionFind()
    timestamp_cover = {}
    weight_edges = sorted(zip(X, edges), reverse=True)
    for x, (n1, n2) in weight_edges:
        t1, g1 = n1.t, n1.g
        t2, g2 = n2.t, n2.g
        p1 = uf.Find(n1)
        p2 = uf.Find(n2)
        if p1 in timestamp_cover:
            cover1 = timestamp_cover[p1]
        else:
            if n1.is_real():
                cover1 = set([t1])
            else:
                cover1 = set(times[:times[times == t1].index[0]])
            timestamp_cover[p1] = cover1
        if p2 in timestamp_cover:
            cover2 = timestamp_cover[p2]
        else:
            if n2.is_real():
                cover2 = set([t2])
            else:
                cover2 = set(times[times[times == t2].index[0]+1:])
            timestamp_cover[p2] = cover2
        if len(cover1.intersection(cover2)) > 0:
            if debug:
                print("ignore", n1, n2, cover1, cover2)
            continue
        if debug:
            print("union", n1, n2, cover1, cover2)
        p = uf.Union(n1, n2)
        timestamp_cover[p].update(cover1)
        timestamp_cover[p].update(cover2)
    # Assign colors to groups.
    tg_color = {}
    partition = sorted(uf.Partition(), key=lambda x: (len(x), x), reverse=True)
    for color_idx, members in enumerate(partition):
        for node in members:
            if not node.is_real():
                continue
            t, g = node.t, node.g
            if t not in tg_color:
                tg_color[t] = {}
            if g in tg_color[t]:
                raise Exception("%s.%s already exists in tg_color %s"%(t, g, tg_color))
            tg_color[t][g] = color_idx + 1
    return tg_color

def color_groups(df, reduce_colors=True,
        t_col='time', g_col='group', i_col='individual'):
    c, A, b, nodes, edges = create_linear_program(df, t_col=t_col, g_col=g_col, i_col=i_col, reduce_colors=reduce_colors)
    sol = solvers.lp(c, A, b)
    if sol['status'] != 'optimal':
        print("Solver error:", sol, file=sys.stderr)
        return None
    tg_color = convert_lp_solution_to_coloring(edges, sol, df[t_col].drop_duplicates())
    return tg_color

def color_dataframe(df,
        sw=1, ab=1, vi=1,
        t_col='time', g_col='group', i_col='individual',
        gcolor_col='gcolor', icolor_col='icolor', reduce_colors=False):
    # Color the groups.
    tg_color = color_groups(df, t_col=t_col, g_col=g_col, i_col=i_col, reduce_colors=reduce_colors)

    # Color the individuals.
    tgi = df.apply(lambda x : (x[t_col], x[g_col], x[i_col]), axis=1)
    total_min_color, it_color = color_individuals(tgi, tg_color, sw=sw, ab=ab, vi=vi)

    # Create a result data frame.
    it_group = dict(df.apply(lambda x: ((x[i_col], x[t_col]), x[g_col]), 1).tolist())
    data = { t_col: [], g_col: [], i_col: [], gcolor_col: [], icolor_col: [] }
    def append(t, g, i, gcolor, icolor):
        data[t_col].append(t)
        data[g_col].append(g)
        data[i_col].append(i)
        data[gcolor_col].append(gcolor)
        data[icolor_col].append(icolor)
    for t in df[t_col].drop_duplicates().tolist():
        for i in df[i_col].drop_duplicates().tolist():
            g = it_group[i, t] if (i, t) in it_group else None
            gcolor = tg_color[t][g] if (i, t) in it_group else None
            icolor = it_color[i, t]
            append(t, g, i, gcolor, icolor)
    return DataFrame(data, columns=[t_col, g_col, i_col, gcolor_col, icolor_col])
