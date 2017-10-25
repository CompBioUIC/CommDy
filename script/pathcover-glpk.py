from itertools import groupby
from pandas import DataFrame
from cvxopt import solvers, sparse, spdiag, matrix, spmatrix

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


def validate_tgi_table():
  # unique g for each (i, t).
  pass

def create_linear_program(df):
  t_min = df.time.min()
  t_max = df.time.max()
  w = {}

  # Creates nodes and edges.
  all_nodes = set()
  for i, g in df.groupby(["individual"]):
    nodes = ["%s.%s"%(t, g) for (t, g) in zip(g.time, g.group)]
    nodes = ["%s.first"%nodes[0]] + nodes + [nodes[-1] + ".last"]
    all_nodes.update(nodes)
    for u, v in zip(nodes, nodes[1:]):
      edge = (u, v)
      if edge in w:
        w[edge] += 1
      else:
        w[edge] = 1
  all_nodes = sorted(all_nodes)
  all_edges = sorted(w)
  n = len(all_nodes)
  m = len(all_edges)
  node_idx = {u: i for i, u in enumerate(all_nodes)}

  # Create constraints.
  vals = []
  rows = []
  cols = []
  for i, (u, v) in enumerate(all_edges):
    vals += [ 1., 1., -1. ]
    rows += [
      node_idx[u],
      n + node_idx[v],
      2 * n + i,
    ]
    cols += [i, i, i]
  A = spmatrix(vals, rows, cols, (2 * n + m, m))
  b = matrix([1.0] * 2 * n + [0.0] * m)
  # objective function.
  c = matrix([float(-w[e]) for e in all_edges])
  #print("A: %s"%A)
  #print("b: %s"%b)
  #print("c: %s"%c)
  return c, A, b, all_nodes, all_edges

def test1():
  tgi = [
    ("t1", "g1", "i1"),
    ("t2", "g1", "i1"),
  ]
  df = DataFrame(data={
    'time': [t for t, g, i in tgi],
    'group': [g for t, g, i in tgi],
    'individual': [i for t, g, i in tgi],
    })
  c, A, b = create_linear_program(df)
  sol = solvers.lp(c, A, b)

def test2():
  tgi = [
    ("t1", "g1", "i1"),
    ("t1", "g1", "i2"),
    ("t1", "g1", "i3"),
    ("t1", "g0", "i4"),
    ("t2", "g1", "i1"),
    ("t2", "g1", "i2"),
    #("t2", "g2", "i3"),
    ("t2", "g0", "i4"),
    ("t3", "g1", "i1"),
    ("t3", "g1", "i2"),
    ("t3", "g1", "i3"),
    ("t3", "g0", "i4"),
  ]
  df = DataFrame(data={
    'time': [t for t, g, i in tgi],
    'group': [g for t, g, i in tgi],
    'individual': [i for t, g, i in tgi],
  })
  c, A, b, nodes, edges = create_linear_program(df)
  sol = solvers.lp(c, A, b)
  print("solution:")
  #for e, x in zip(edges, sol['x']):
  #  print("%s %f"%(e, x))
  uf = UnionFind()
  timestamp_cover = {}
  for x, (n1, n2) in sorted(zip(sol['x'], edges), reverse=True):
    if n1.endswith('first') or n2.endswith('last'):
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
      print("ignore", (t1, g1), (t2, g2), cover1, cover2)
      continue
    print("union", (t1, g1), (t2, g2), cover1, cover2)
    p = uf.Union(n1, n2)
    timestamp_cover[p].update([t1, t2])

  # Assign colors to groups.
  tg_color = {}
  for color_idx, members in enumerate(sorted(uf.Partition(), key=lambda x: (len(x), x), reverse=True)):
    for tg in members:
      t, g = tg.split('.')
      tg_color[(t, g)] = color_idx + 1
  group_color = []
  for t, g, _ in tgi:
    group_color.append(tg_color[(t, g)])
  print(DataFrame(data={
    'time': [t for t, g, i in tgi],
    'group': [g for t, g, i in tgi],
    'individual': [i for t, g, i in tgi],
    'group_color': group_color,
  }))

  # Color the individuals.
  for i in df.individual.unique():
    subrows = df[df.individual == i]
    for t in df.time.unique():
      g = df[(df.individual == i) & (df.time == t)].group.values
      if len(g) > 0:
        g = g[0]
        gc = tg_color[(t, g)]
      else:
        g = None
        gc = None
      other_group_colors = set(tg_color[(t, g)] for g in df[(df.time == t) & (df.group != g)].group.unique())
      colors = sorted([] if gc is None else [gc]  + list(other_group_colors))
      print(i, t, g, gc, other_group_colors, colors)

      #print(i, t, tg_color[(t, g)])
    #print(list(zip(subrows.time, subrows.group, subrows.individual)))
    #for _, r in subrows.iterrows():
    #  print((r.time, r.group))
    #print

def main():
  test2()

# Test.
if __name__ == '__main__':
    main()
