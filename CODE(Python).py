s = input()

prevs = dict()
a = []
for i in range(len(s)):
    if s[i] in prevs:
        a.append(i - prevs[s[i]])
    else:
        a.append(i + 1)
    prevs[s[i]] = i

class Node:
    def __init__(self, **d):
        self.__dict__ = d

class Edge:
    def __init__(self, **d):
        self.__dict__ = d

root = Node(edges = dict(), depth = 0, slink = None)
edge0 = Edge(a = root, b = root, u = 0, l = 0)
cur = (edge0, 0, 0)
leaves = 0
ans = 0

def conv(c, depth):
    return -1 if c > depth else c

def simplify(cur):
    edge, u, l = cur
    while l > edge.l:
        c = conv(a[u + edge.l], edge.a.depth + edge.l)
        edge, u, l = edge.b.edges[c], u + edge.l, l - edge.l
    return edge, u, l

def toStr(a, depth):
    l = []
    for i in range(len(a)):
        l.append(conv(a[i], depth + i))
    return map(str, l)

def printTree(edge, tabs = ''):
    print(tabs + ':'.join(toStr(a[edge.u:edge.u+edge.l], edge.a.depth)), 
    edge.b.depth, edge.b.slink)
    for e in edge.b.edges.values():
        printTree(e, tabs + '  ')

def slink(cur):
    edge, u, l = cur
    if edge.a == root:
        assert(edge != edge0)
        return simplify((edge0, u + 1, l - 1))
    else:
        edge.a.slink = simplify(edge.a.slink)
        e1, u1, l1 = edge.a.slink
        return simplify((e1, u - l1, l + l1))

#print(':'.join(map(str, a)))

for i in range(len(s)):
    while True:
        edge, u, l = cur
        c = conv(a[i], edge.a.depth + l)
        if l == edge.l:
            if c in edge.b.edges:
                break
            leaves += 1
            leaf = Node(depth = -1, slink = None, edges = dict())
            edge.b.edges[c] = Edge(a = edge.b, b = leaf, u = i, l = len(s) - i)
        else:
            c1 = conv(a[edge.u + l], edge.a.depth + l)
            if c == c1:
                break
            leaves += 1
            leaf = Node(depth = -1, slink = None, edges = dict())
            branch = Node(edges = dict(), depth = edge.a.depth + l, slink = slink(cur))
            branch.edges[c] = Edge(a = branch, b = leaf, u = i, l = len(s) - i)
            branch.edges[c1] = Edge(a = branch, b = edge.b, u = edge.u + l, l = edge.l - l)
            edge.b = branch
            edge.l = l
        if edge == edge0 and l == 0:
            cur = None
            break
        if edge.a == root:
            assert(edge != edge0)
            cur = simplify((edge0, u + 1, l - 1))
        else:
            edge.a.slink = simplify(edge.a.slink)
            e1, u1, l1 = edge.a.slink
            cur = simplify((e1, u - l1, l + l1))
    if cur == None:
        cur = edge0, i + 1, 0
    else:
        edge, u, l = cur
        assert(u + l == i)
        cur = simplify((edge, u, l + 1))
    ans += leaves
    print(ans)
    #printTree(edge0)