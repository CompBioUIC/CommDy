#!/usr/bin/python

import sys,re

from GtmFile import *
from GlpxOutput import *
from GroupGraph import *
import sys,re,os

gtm_fname = None
pc_fname = None
limit=None
quiet=False
verbose=False

# Parses command-line flags.
for arg in sys.argv[1:]:
  p = re.compile('-.+')
  # If arg is a flag.
  if p.match(arg):
    # Limits to a single iteration of path covering.
    if arg == '-1':
      limit=1
    elif arg == '-v':
      verbose=True
    elif arg == '-q':
      quiet=True
    else:
      print "Unknown option", arg
      exit(1)
  # Else, arg is a filename.
  else:
    # If arg is a GTM filename.
    if arg.endswith('.gtm') or arg.endswith('.gtm2'):
      gtm_fname = arg
    # If arg is a path-cover filename (i.e. the output of a previous run).
    elif arg.endswith('.out'):
      pc_fname = arg
    else:
      print 'Unknown file extension:', arg
      exit(1)

# If no GTM file is given, try to guess it.
if gtm_fname is None:
  gtm_fname = guess_gtmfile()
if gtm_fname is None:
  print "Cannot guess a GTM file"
  exit(1)
# Reads the GTM file.
gtm = GtmFile(gtm_fname)

# Creates mapping: group ID -> members.
for g,s in gtm.group.iteritems():
  gtm.group[g] = set(s)

# Creates a group graph from the GTM.
group_graph = GroupGraph(gtm)
group = group_graph.group
group_time = group_graph.group_time

# Generates unique tmp filenames.
bname = os.path.splitext(os.path.basename(gtm_fname))[0]
pid=os.getpid()
prefix = os.path.dirname(gtm_fname)
if prefix != "": prefix += os.path.sep
out_fname = prefix + "%s-ipc.glpk.%d.out" %(bname,pid)
gms_fname = prefix + "%s-ipc.glpk.%d.mod" %(bname,pid)
log_fname = prefix + "%s-ipc.glpk.%d.log" %(bname,pid)

pc_edges=[]
if pc_fname is not None:
  # read the given pathcover file (i.e. the output of a previous run).
  f = open(pc_fname, 'r')
  for line in f.readlines():
    line = line.strip()
    fields = map(int, line.split(','))
    if len(fields)==3:
      # An edge from u to v (with weight w, for backward compatibility).
      u,v,w = fields
    elif len(fields)==2:
      # An edge from u to v.
      u,v = fields
    else: raise
    pc_edges.append((u,v))

# Starts the main iteration, each finds a path cover on the group graph.
it_count=0
while limit is None or it_count<limit:
  it_count+=1
  sys.stderr.write("iteration %d: "%it_count)

  #sys.stderr.write("assign parents\n")

  # Finds the parent node of each node. The parent is defined as the incoming
  # neighbor in the subgraph induced by the current path cover.
  parent = {}
  for e in pc_edges:
    u,v = e
    if group_graph.group_time[u]>group_graph.group_time[v]:
      print "u", u, group_graph.group_time[u], 
      print "v", v, group_graph.group_time[v]
      raise Exception("Found an edge going back in time")
    if v in parent: 
      print "u", u, "t", group_graph.group_time[u]
      print "v", v, "t", group_graph.group_time[v]
      print "parent[v]", parent[v], "t", group_graph.group_time[parent[v]]
      raise Exception("Input is not a path cover. "
          +"Vertex %s has two parents: %s and %s."%(v, parent[v], u))
    parent[v] = u

  ## debug print
  #for g in range(1, len(group_graph.groups)+1):
  # print "group", g, "time", group_time[g], "members:",
  # for i in sorted(group[g]):
  #   print i,
  # print

  #outedge = {}
  #inedge = {}
  #for index,e in enumerate(edges):
  # (g,h) = e
  # if g not in outedge: outedge[g]=[]
  # outedge[g].append((g,h))
  # if h not in inedge: inedge[h]=[]
  # inedge[h].append((g,h))

  # Prepares to create a path graph
  first_nodes = []  # Collection of nodes which start paths.
  first_node = {}   # first_node[g] starts the path containing g.
  last_node = {}    # last_node[g] ends the path containing g.

  #sys.stderr.write("find paths\n")

  # For each group, in chronological order:
  for g in group_graph.groups_intime:
    if g in parent and parent[g] != g: # possibly ending node of a path
      parent_g = parent[g]
      first_group_g = \
          first_node[parent_g] if parent_g in first_node else \
          parent_g
      first_node[g] = first_group_g
      last_node[first_group_g] = g
    else: # starting node of a path
      first_node[g] = g
      last_node[g] = g
      first_nodes.append(g)

  #sys.stderr.write("create path graph: %d paths\n"%(len(first_nodes)))

  # create path graph
  edges = []
  progress=0
  # For each path p that ends at group g, we add edges to groups h for which
  # the members of g leave, preferring those nearer in time.
  for index,p in enumerate(first_nodes):
    g = last_node[p]
    members_g = set(group_graph.group[g])
    #if progress!=g*100/len(first_nodes):
    # progress=g*100/len(first_nodes)
    # sys.stderr.write("progress %d%%\n"%(progress))
    #g_index = group_graph.groups_intime.index(g)

    # For each group h that comes after g.
    for h in first_nodes[index+1:]:
      if group_graph.group_time[h]<=group_graph.group_time[g]:
        # We skip those where time(h) <= time(g).
        continue
      h_inter = members_g & set(group_graph.group[h])
      if len(h_inter)>0:
        edges.append((p, h, len(h_inter)))
        # Removes the members accounted for by the new edge.
        members_g = members_g - h_inter
        if len(members_g)==0: break

  if not edges:
    sys.stderr.write("no edges\n")
    break

  # write gms file
  f = open(gms_fname, 'w')
  f.write("# generated by ipc-glpk.py\n")
  f.write("set V;\n")
  f.write("param w{V,V}>=0;\n")
  f.write("var x{V,V}>=0;\n")
  f.write("maximize total_w: sum{i in V, j in V} w[i,j] * x[i,j];\n")
  f.write("s.t. in_degree {j in V}: sum{i in V} x[i,j] <= 1;\n")
  f.write("    out_degree {i in V}: sum{j in V} x[i,j] <= 1;\n")
  f.write("data;\n")

  n = len(first_nodes)
  m = len(edges)
  f.write("set V :=")
  for p in first_nodes:
    f.write(" %d"%p)
  f.write(";\n")

  f.write("param w default 0, \n")
  for index,e in enumerate(edges):
    (g,h,w) = e
    f.write(" %d %d %d"%(g, h, w))
    if index % 5 == 4 or index==m-1: f.write("\n")
  f.write(";\n")

  f.write("end;\n")

  f.close()

  # run path-cover
  #rc = os.system("gams %s"%(gms_fname))
  if verbose:
    rc = os.system("glpsol -m %s -o %s 2>&1 | tee %s"%\
            (gms_fname, out_fname, log_fname))
  else:
    rc = os.system("glpsol -m %s -o %s > %s"%\
            (gms_fname, out_fname, log_fname))
  if rc!=0: raise Exception("System call return %d"%rc)

  # read output
  out = GlpxOutput(out_fname)
  pcpc_edges=out.edges
  sum_w=out.obj_value
  sys.stderr.write("%d\n"%sum_w)

  ## debug
  #for u,v,w in sorted(pcpc_edges):
  # print "%d,%d,%d"%(u,v,w)

  if pcpc_edges:
    for e in pcpc_edges:
      u,v = e
      if u not in last_node:
        print "u", u
        print "paths", first_nodes
        print "last_node", last_node
      pc_edges.append((last_node[u], v))
  else:
    break

sys.stderr.write("clean up\n")
#sys.stderr.write("rm %s-ipc.glpk.%d.*\n"%(bname,pid))
os.system("rm " + prefix + "%s-ipc.glpk.%d.*" %(bname,pid))

#output
if limit==1:
  out_fname = prefix + "%s_pc.out" % bname
  gcolor_fname = prefix + "%s_pc.gcolor" % bname
  color2_fname = prefix + "%s_pc-c111.color2" % bname
  png_fname = prefix + "%s_pc-c111.png" % bname
else:
  out_fname = prefix + "%s_ipc.out" % bname
  gcolor_fname = prefix + "%s_ipc.gcolor" % bname
  color2_fname = prefix + "%s_ipc-c111.color2" % bname
  png_fname = prefix + "%s_ipc-c111.png" % bname
if not quiet:
  sys.stderr.write("(What's next?)\n")
  sys.stderr.write("gamsout2color.py %s >%s\n"\
    %(out_fname, gcolor_fname))
  sys.stderr.write("color_ind2 %s <%s >%s\n"\
    %(gtm_fname, gcolor_fname, color2_fname))
  #sys.stderr.write("color2dot %s %s -Tpng -o %s -ranksep 1\n"\
  # %(gtm_fname, color2_fname, png_fname))
  #sys.stderr.write("color2dot %s %s -Tpng -wsize 100\n"\
  # %(gtm_fname, color2_fname))
  sys.stderr.write("color3dot.py %s\n"%(color2_fname))

f = open(out_fname, 'w')
for e in sorted(pc_edges):
  u,v = e
  f.write("%d %d\n"%(u,v))
f.close()

