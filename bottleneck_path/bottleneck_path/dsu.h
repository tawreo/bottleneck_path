#ifndef DSU_H
#define DSU_H

#include <vector>
#include <algorithm>

class DSU {
private:
    std::vector<int> parent;
    std::vector<int> rank;

public:
    DSU(int n) {
        parent.resize(n);
        rank.resize(n, 0);
        for (int i = 0; i < n; i++) {
            parent[i] = i;
        }
    }

    int find(int x) {
        if (parent[x] != x) {
            parent[x] = find(parent[x]);  // Path compression
        }
        return parent[x];
    }

    bool unite(int x, int y) {
        int rx = find(x);
        int ry = find(y);
        if (rx == ry) return false;

        // Union by rank
        if (rank[rx] < rank[ry]) {
            parent[rx] = ry;
        }
        else if (rank[rx] > rank[ry]) {
            parent[ry] = rx;
        }
        else {
            parent[ry] = rx;
            rank[rx]++;
        }
        return true;
    }

    bool connected(int x, int y) {
        return find(x) == find(y);
    }
};

#endif