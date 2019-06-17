#!/home/haussmae/pypy/pypy/bin/pypy
import collections
from collections import deque
import sys
# A small python program to build a graph and
# perform DFS/DFS on it. The input is a file
# with pairs of lines consisting of parent and
# child, each identified using a string.

def bfs(g, start):
    queue, enqueued = deque([(None, start)]), set([start])
    while queue:
        parent, n = queue.popleft()
        yield parent, n
        new = set(g[n]) - enqueued
        enqueued |= new
        queue.extend([(n, child) for child in new])

def somePaths(g, start, end):
    """Opposed to the allPaths function, when visiting
    a node put it into the enqueued set to mark it visited.
    This does not fine all paths, but if one exists it will
    be found."""
    queue, enqueued = deque([[start]]), set([start])
    while queue:
        path = queue.popleft()
        #print "Path:" +str(path)
        lastNode = path[len(path)-1]
        #print "LastNode: " + lastNode
        if (lastNode == end):
          yield path
        nextNodes = set(g[lastNode].keys()) - enqueued
        #print "NextNodes:"
        #print nextNodes
        enqueued |= nextNodes
        for child in nextNodes:
          if child not in path:
            nextPath = [] 
            nextPath = path + [child]
            queue.extend([nextPath])

def allPaths(g, start, end):
    queue, enqueued = deque([[start]]), set([start])
    while queue:
        path = queue.popleft()
        #print "Path:" +str(path)
        lastNode = path[len(path)-1]
        #print "LastNode: " + lastNode
        if (lastNode == end):
          yield path
        nextNodes = set(g[lastNode].keys())# - enqueued
        #print "NextNodes:"
        #print nextNodes
        #enqueued |= nextNodes
        for child in nextNodes:
          if child not in path:
            nextPath = [] 
            nextPath = path + [child]
            queue.extend([nextPath])

def dfs(g, start):
    stack, enqueued = [(None, start)], set([start])
    while stack:
        parent, n = stack.pop()
        yield parent, n
        new = set(g[n]) - enqueued
        enqueued |= new
        stack.extend([(n, child) for child in new])

def shortest_path(g, start, end):
    parents = {}
    for parent, child in bfs(g, start):
        parents[child] = parent
        if child == end:
            revpath = [end]
            while True:
                parent = parents[child]
                revpath.append(parent)
                if parent == start:
                    break
                child = parent
            return list(reversed(revpath))
    return None # or raise appropriate exception


def getIndex(category, categoryMap, nextIndex):
  """ Get the index of category from categoryMap,
  if it is not present use nextIndex. Returns the
  found index, and new value for nextIndex"""
  if category not in categoryMap:
    categoryMap[category] = nextIndex;
    nextIndex += 1
  return categoryMap[category], nextIndex

def printUsage():
  print """Usage: graph.py <inputFile> <from> <to>  [\"node1,node2...\"]\n
  The inputFile consists of a line for the parent followed by a line
  for the child. All paths from the node <from> to the node <to> are computed
  in a BFS-manner (can take a long time). The fourth, optional parameter 
  is a list of nodes to be ignored when building the graph.\n"""


if __name__ == '__main__':
    if len(sys.argv) < 3:
      printUsage()
      exit(1)
    # A map from string to index.
    cMap = {}
    # A map from index to string.
    rMap = {}
    nextIndex = 0
    nEdges = 0
    inputFile = open (sys.argv[1], "r")
    # The graph is a hashmap of hashmaps, mapping
    # from parent -> {child1 -> 0, child2 -> 0 ...}.
    graph = collections.defaultdict(dict)
    toRemove = []
    if len(sys.argv)==5:
      toRemove = sys.argv[4].split(',')
    print "Reading input..."
    while (True):
      parent = inputFile.readline().strip()
      # EOF? -> break
      if not parent:
        break
      # We assume each parent starts with :ct:
      # Continue reading until we find the next parent.
      while not parent.startswith(":ct:"):
        parent = inputFile.readline()
      # EOF? -> break
        if not parent:
          break
      # Only use string starting at the last ':'.
      parent = parent[parent.rfind(':')+1:]
      if parent in toRemove:
        continue
      child = inputFile.readline().strip()
      # EOF? -> break
      if not child:
        break
      # Only use string starting at the last ':'.
      child = child[child.rfind(':')+1:].strip()
      # Use indices in hashmaps.
      pIndex, nextIndex =  getIndex(parent, cMap, nextIndex)
      cIndex, nextIndex =  getIndex(child, cMap, nextIndex)
      graph[pIndex][cIndex] = 0
      nEdges += 1
    for k, v in cMap.iteritems():
      rMap[v] = k
    # Get the source node index.
    sIndex, nextIndex = getIndex(sys.argv[2], cMap, nextIndex)
    print "Done."
    print "Number of nodes: " + str(nextIndex)
    print "Number of edges: " + str(nEdges)
    print "Direct children of " + sys.argv[2]
    print graph[sIndex]
    # Get the target node index.
    tIndex, nextIndex = getIndex(sys.argv[3], cMap, nextIndex)
    print "Direct children of " + sys.argv[3]
    print graph[tIndex]
    # allPaths is a generator function, so we can iterator over it.
    for path in somePaths(graph, sIndex, tIndex):
      sys.stdout.write( "Path("+str(len(path))+"): ")
      for node in path:
        sys.stdout.write(rMap[node] + " ")
      print
