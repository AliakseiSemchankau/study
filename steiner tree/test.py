import random
import math
import sys

import networkx as nx
import matplotlib.pyplot as plt
import pylab


def euclidean_distance(a, b):
    return math.sqrt(float((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2))


def manhattan_distance(a, b):
    return abs(a[0] - b[0]) + abs(a[1] - b[1])


def get_distance_from_triple(v, triple, g):
    return g[v][triple[0]]['weight'] + g[v][triple[1]]['weight'] + g[v][triple[2]]['weight']


def get_weight(g):
    result = 0
    for i in g.nodes():
        for j in g.nodes():
            if g.has_edge(i, j) and j > i:
                result += g[i][j]['weight']
    return result


def read_test_data():
    points_x = []
    points_y = []
    terminals = []

    f = open("input.txt")
    for i in range(8):
        f.readline()
    number_of_nodes = int((f.readline().split())[1])
    number_of_edges = int((f.readline().split())[1])

    g = nx.Graph()
    for i in range(number_of_nodes):
        g.add_node(i)

    for i in range(number_of_edges):
        temp = f.readline().split()
        g.add_edge(int(temp[1]) - 1, int(temp[2]) - 1, weight=int(temp[3]))
    for i in range(3):
        f.readline()

    number_of_terminals = int(f.readline().split()[1])
    for i in range(number_of_terminals):
        terminals.append(int(f.readline().split()[1]) - 1)
    for i in range(3):
        f.readline()
    for i in range(number_of_nodes):
        temp = f.readline().split()
        points_x.append(int(temp[2]))
        points_y.append(int(temp[3]))

    length_shortest_paths = nx.all_pairs_dijkstra_path_length(g)
    shortest_paths = nx.all_pairs_shortest_path(g)

    metric_closure = nx.Graph()
    for i in range(number_of_nodes):
        metric_closure.add_node(i)
    for i in range(number_of_nodes):
        for j in range(number_of_nodes):
            metric_closure.add_edge(i, j, weight=length_shortest_paths[i][j])

    print metric_closure
    return (terminals, shortest_paths, metric_closure, points_x, points_y, g)


def initialize_graph(size):
    points_x = []
    points_y = []
    terminals = []
    for i in range(size):
        points_x.append(random.uniform(-100, 100))
        points_y.append(random.uniform(-100, 100))
    for i in range(size):
        if i % 2 == 0:
            terminals.append(i)

    h = nx.erdos_renyi_graph(size, 0.3)
    g = nx.Graph()
    for i in range(size):
        g.add_node(i)

    for i in range(size):
        for j in range(i, size):
            if h.has_edge(i, j):
                g.add_edge(i, j)
                # weight=manhattan_distance((points_x[i], points_y[i]), (points_x[j], points_y[j])))

    length_shortest_paths = nx.all_pairs_dijkstra_path_length(g)
    shortest_paths = nx.all_pairs_shortest_path(g)

    metric_closure = nx.Graph()
    for i in range(size):
        metric_closure.add_node(i)
    for i in range(size):
        for j in range(size):
            metric_closure.add_edge(i, j, weight=length_shortest_paths[i][j])

    print metric_closure
    return (terminals, shortest_paths, metric_closure, points_x, points_y, g)


def draw(g, points_x, points_y):
    size = len(g)
    for i in range(size):
        for j in range(size):
            if g.has_edge(i, j):
                plt.plot([points_x[i], points_x[j]], [points_y[i], points_y[j]], "k-")
    plt.show()


def test():
    data = read_test_data()
    g = data[2]
    shortest_paths = data[1]

    terminals = data[0]
    points_x = data[3]
    points_y = data[4]
    input_graph = data[5]
    size = len(points_x)

    f = g.subgraph(terminals)

    triples = []
    for i in range(len(terminals)):
        for j in range(i, len(terminals)):
            for k in range(j, len(terminals)):
                if i != j and i != k and k != j:
                    triples.append((terminals[i], terminals[j], terminals[k]))

    v = {}
    d = {}

    min_dist = sys.float_info.max
    vertex_min = 0
    for tr in triples:
        for k in range(size):
            if k != tr[0] and k != tr[1] and k != tr[2]:
                temp = get_distance_from_triple(k, tr, g)
                if temp < min_dist:
                    min_dist = temp
                    vertex_min = k
        v[tr] = vertex_min
        d[tr] = min_dist

    index_win = 0
    contraction_graph = f
    w = []

    while (1):
        max_win = -sys.float_info.max
        for tr in triples:
            temp = f.copy()
            temp.remove_edge(tr[0], tr[1])
            temp.remove_edge(tr[1], tr[2])
            temp.add_edge(tr[0], tr[1], weight=0)
            temp.add_edge(tr[1], tr[2], weight=0)

            win = get_weight(nx.minimum_spanning_tree(f)) - get_weight(nx.minimum_spanning_tree(temp)) - d[tr]

            if win > max_win:
                max_win = win
                index_win = tr
                contraction_graph = temp

        if max_win <= 0:
            break
        f = contraction_graph
        w.append(v[index_win])

    s = terminals + w

    temp = g.subgraph(s)

    t = nx.minimum_spanning_tree(temp)

    h = nx.Graph()
    for i in range(size):
        h.add_node(i)

    for i in range(size):
        for j in range(size):
            if t.has_edge(i, j):
                path = shortest_paths[i][j]
                for l in range(len(path) - 1):
                    h.add_edge(path[l], path[l + 1])
    # edges

    # nx.draw_networkx_edges(g,pos,width=1)

    h = nx.minimum_spanning_tree(h)
    edges_in_shortest_paths_between_terminals = set()
    for i in terminals:
        paths_from_node = nx.single_source_shortest_path(h,i)
        for k in paths_from_node.keys():
            path = paths_from_node[k]
            for j in range(len(path) - 1):
                edges_in_shortest_paths_between_terminals.add((path[j], path[j + 1]))

    for i in range(size):
        for j in range(size):
            if h.has_edge(i,j) and (i,j) not in edges_in_shortest_paths_between_terminals:
                h.remove_edge(i,j)

    plt.plot(points_x, points_y, "bo")
    for i in terminals:
        plt.plot(points_x[i], points_y[i], "rs")

    for i in range(size):
        for j in range(size):
            if input_graph.has_edge(i, j):
                plt.plot([points_x[i], points_x[j]], [points_y[i], points_y[j]], linewidth = 1, color = 'b')

    for i in range(size):
        for j in range(size):
            if h.has_edge(i, j):
                plt.plot([points_x[i], points_x[j]], [points_y[i], points_y[j]], linewidth = 2, color ='r')
    plt.autoscale(tight=True)
    # pylab.xlim(-10,10)
    plt.show()


def main():
    test()


if __name__ == "__main__":
    main()
