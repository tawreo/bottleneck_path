#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <tuple>
#include <algorithm>
#include <random>
#include <chrono>
#include <set>
#include <queue>
#include <iostream>
#include <cstdlib>

struct Edge {
    int from, to, weight;
    Edge(int f, int t, int w) : from(f), to(t), weight(w) {}

    bool operator<(const Edge& other) const {
        return weight < other.weight;
    }

    bool operator>(const Edge& other) const {
        return weight > other.weight;
    }
};

class Graph {
public:
    int n;  // number of vertices
    std::vector<Edge> edges;
    std::vector<std::vector<std::pair<int, int>>> adj;  // adjacency list: to, weight

    // Default constructor
    Graph() : n(0) {}

    // Constructor with number of vertices
    Graph(int vertices) : n(vertices) {
        adj.resize(n);
    }

    void addEdge(int u, int v, int w) {
        edges.emplace_back(u, v, w);
        adj[u].emplace_back(v, w);
        adj[v].emplace_back(u, w);
    }

    // Get edge count
    int m() const {
        return (int)edges.size();
    }

    // Generate complete graph with random weights (1..100)
    static Graph generateComplete(int vertices, unsigned seed = 42) {
        Graph g(vertices);
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> dist(1, 100);

        for (int i = 0; i < vertices; i++) {
            for (int j = i + 1; j < vertices; j++) {
                g.addEdge(i, j, dist(rng));
            }
        }
        return g;
    }

    // Generate sparse graph (m ≈ 5*n) with random weights
    // Generate sparse graph (m ≈ 5*n) with random weights
    static Graph generateSparse(int vertices, unsigned seed = 42) {
        Graph g(vertices);
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> dist(1, 100);

        int targetEdges = 5 * vertices;
        int edgesAdded = 0;

        // Исправленная вероятность: добавляем рёбра пока не наберём targetEdges
        // Проходим по всем парам вершин, но с большей вероятностью
        for (int i = 0; i < vertices && edgesAdded < targetEdges; i++) {
            for (int j = i + 1; j < vertices && edgesAdded < targetEdges; j++) {
                // Увеличиваем вероятность до ~10%
                if (rand() % 100 < 10) {  // 10% шанс добавить ребро
                    g.addEdge(i, j, dist(rng));
                    edgesAdded++;
                }
            }
        }

        // Ensure graph is connected by adding a spanning tree
        for (int i = 1; i < vertices; i++) {
            bool edgeExists = false;
            for (const auto& e : g.edges) {
                if ((e.from == 0 && e.to == i) || (e.from == i && e.to == 0)) {
                    edgeExists = true;
                    break;
                }
            }
            if (!edgeExists) {
                g.addEdge(0, i, dist(rng));
                edgesAdded++;
            }
        }

        return g;
    }
};

#endif