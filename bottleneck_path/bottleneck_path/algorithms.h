#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "graph.h"
#include "dsu.h"
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <cmath>
#include <climits>
#include <queue>
#include <random>
#include <ctime>
#include <set>
#include <map>

// ============================================
// 1. BASELINE ALGORITHM (Camerini's original approach)
// ============================================

namespace Baseline {

    // Вспомогательная функция для поиска новой метки вершины после контракции
    int getComponentId(const std::vector<int>& vertexMap, int v) {
        return vertexMap[v];
    }

    int cameriniBPP(const Graph& g, int s, int t) {
        if (s == t) return INT_MAX;

        Graph current = g;
        int currentS = s;
        int currentT = t;
        int answer = 0;

        // Защита от бесконечного цикла
        int maxIterations = 100;
        int iteration = 0;
        int prevEdges = current.edges.size();

        while (current.edges.size() > 0 && current.n > 1 && iteration < maxIterations) {
            iteration++;

            // Собираем уникальные веса
            std::vector<int> weights;
            for (const Edge& e : current.edges) {
                weights.push_back(e.weight);
            }

            if (weights.empty()) break;

            // Находим медиану
            size_t mid = weights.size() / 2;
            std::nth_element(weights.begin(), weights.begin() + mid, weights.end());
            int median = weights[mid];

            // Проверяем, соединяет ли порог median s и t
            DSU dsu(current.n);
            for (const Edge& e : current.edges) {
                if (e.weight >= median) {
                    dsu.unite(e.from, e.to);
                }
            }

            bool connected = dsu.connected(currentS, currentT);

            if (connected) {
                // Оставляем только рёбра с весом >= median
                answer = median;
                Graph newGraph(current.n);
                for (const Edge& e : current.edges) {
                    if (e.weight >= median) {
                        newGraph.addEdge(e.from, e.to, e.weight);
                    }
                }
                current = newGraph;
            }
            else {
                // Контракция компонент, связанных рёбрами с весом > median
                DSU componentDSU(current.n);
                for (const Edge& e : current.edges) {
                    if (e.weight > median) {
                        componentDSU.unite(e.from, e.to);
                    }
                }

                // Строим отображение вершин в компоненты
                std::vector<int> componentId(current.n, -1);
                int componentCount = 0;

                for (int i = 0; i < current.n; i++) {
                    int root = componentDSU.find(i);
                    if (componentId[root] == -1) {
                        componentId[root] = componentCount++;
                    }
                }

                // Если всего одна компонента - выходим
                if (componentCount <= 1) {
                    break;
                }

                // Создаём маппинг для каждой вершины
                std::vector<int> vertexToComp(current.n);
                for (int i = 0; i < current.n; i++) {
                    int root = componentDSU.find(i);
                    vertexToComp[i] = componentId[root];
                }

                // Создаём контрагированный граф
                Graph contracted(componentCount);
                std::set<std::pair<int, int>> addedEdges;

                for (const Edge& e : current.edges) {
                    int uComp = vertexToComp[e.from];
                    int vComp = vertexToComp[e.to];
                    if (uComp != vComp) {
                        auto edgeKey = std::make_pair(
                            std::min(uComp, vComp),
                            std::max(uComp, vComp)
                        );
                        if (addedEdges.find(edgeKey) == addedEdges.end()) {
                            contracted.addEdge(uComp, vComp, e.weight);
                            addedEdges.insert(edgeKey);
                        }
                    }
                }

                // Обновляем currentS и currentT
                currentS = vertexToComp[currentS];
                currentT = vertexToComp[currentT];
                current = contracted;
            }

            // Проверка на зацикливание
            if (current.edges.size() == prevEdges) {
                break;
            }
            prevEdges = current.edges.size();
        }

        return answer;
    }

    std::vector<int> solve(const Graph& g,
        const std::vector<std::pair<int, int>>& pairs) {
        std::vector<int> answers;
        for (const auto& p : pairs) {
            answers.push_back(cameriniBPP(g, p.first, p.second));
        }
        return answers;
    }
}

// ============================================
// 2. OFFLINE MPBPP ALGORITHM (Algorithm 3 from the paper)
// Исправленная версия
// ============================================

namespace Offline {

    std::vector<int> solve(const Graph& g,
        const std::vector<std::pair<int, int>>& pairs) {
        int n = g.n;
        int k = (int)pairs.size();

        std::vector<int> answer(k, -1);
        std::vector<std::unordered_set<int>> compInd(n);
        DSU dsu(n);

        // Инициализация
        for (int i = 0; i < k; i++) {
            int s = pairs[i].first;
            int t = pairs[i].second;
            if (s == t) {
                answer[i] = INT_MAX;
            }
            else {
                compInd[s].insert(i);
                compInd[t].insert(i);
            }
        }

        // Сортируем рёбра по убыванию
        std::vector<Edge> sortedEdges = g.edges;
        std::sort(sortedEdges.begin(), sortedEdges.end(),
            [](const Edge& a, const Edge& b) {
                return a.weight > b.weight;
            });

        // Обрабатываем рёбра
        for (const Edge& e : sortedEdges) {
            int rootU = dsu.find(e.from);
            int rootV = dsu.find(e.to);

            if (rootU != rootV) {
                // !!! КЛЮЧЕВОЕ ИСПРАВЛЕНИЕ !!!
                // Всегда работаем с меньшим множеством
                if (compInd[rootU].size() < compInd[rootV].size()) {
                    std::swap(rootU, rootV);
                }

                // Находим пересечение (итерируемся по меньшему)
                std::vector<int> toAnswer;
                for (int queryId : compInd[rootV]) {
                    if (compInd[rootU].count(queryId)) {
                        toAnswer.push_back(queryId);
                    }
                }

                // Отвечаем на запросы
                for (int queryId : toAnswer) {
                    if (answer[queryId] == -1) {
                        answer[queryId] = e.weight;
                    }
                    compInd[rootU].erase(queryId);
                }

                // Перемещаем оставшиеся из меньшего в большее
                for (int queryId : compInd[rootV]) {
                    compInd[rootU].insert(queryId);
                }
                compInd[rootV].clear();

                // Объединяем компоненты
                dsu.unite(rootU, rootV);

                // Обновляем корень (важно для последующих операций)
                int newRoot = dsu.find(rootU);
                if (newRoot != rootU) {
                    for (int queryId : compInd[rootU]) {
                        compInd[newRoot].insert(queryId);
                    }
                    compInd[rootU].clear();
                }
            }
        }

        // Неотвеченные запросы (вершины не соединены)
        for (int i = 0; i < k; i++) {
            if (answer[i] == -1) {
                answer[i] = 0;  // или INT_MIN, в зависимости от контекста
            }
        }

        return answer;
    }
}   

// ============================================
// 3. ONLINE MPBPP ALGORITHM (Algorithms 4, 5, 6 from the paper)
// ============================================

namespace Online {

    // Построение максимального остовного дерева (MaxST)
    Graph buildMaxST(const Graph& g) {
        if (g.n == 0) return Graph(0);

        Graph mst(g.n);
        std::vector<bool> inMST(g.n, false);
        using Element = std::tuple<int, int, int>; // weight, vertex, parent
        std::priority_queue<Element> pq;

        int startVertex = 0;
        for (int i = 0; i < g.n; i++) {
            if (!g.adj[i].empty()) {
                startVertex = i;
                break;
            }
        }
        inMST[startVertex] = true;

        for (const auto& edge : g.adj[0]) {
            pq.push(std::make_tuple(edge.second, edge.first, 0));
        }

        while (!pq.empty()) {
            auto [weight, v, parent] = pq.top();
            pq.pop();

            if (inMST[v]) continue;

            inMST[v] = true;
            mst.addEdge(parent, v, weight);

            for (const auto& edge : g.adj[v]) {
                if (!inMST[edge.first]) {
                    pq.push(std::make_tuple(edge.second, edge.first, v));
                }
            }
        }

        return mst;
    }

    class LCAPreprocessor {
    private:
        int n;
        int LOG;
        std::vector<std::vector<int>> parent;
        std::vector<std::vector<int>> minEdge;
        std::vector<int> depth;
        int root;

    public:
        LCAPreprocessor() : n(0), LOG(0), root(0) {}

        void preprocess(const Graph& tree, int r) {
            root = r;
            n = tree.n;
            if (n == 0) return;

            depth.assign(n, 0);
            std::vector<int> parentWeight(n, 0);
            std::vector<int> par(n, -1);

            // BFS from root
            std::queue<int> q;
            q.push(root);
            par[root] = root;
            depth[root] = 0;

            while (!q.empty()) {
                int v = q.front();
                q.pop();

                for (const auto& edge : tree.adj[v]) {
                    int to = edge.first;
                    int w = edge.second;
                    if (to != par[v]) {
                        par[to] = v;
                        parentWeight[to] = w;
                        depth[to] = depth[v] + 1;
                        q.push(to);
                    }
                }
            }

            // Вычисляем максимальную глубину и LOG
            int maxDepth = 0;
            for (int d : depth) {
                if (d > maxDepth) maxDepth = d;
            }
            LOG = (maxDepth == 0) ? 1 : (int)std::log2(maxDepth) + 2;

            // Инициализация таблиц
            parent.assign(n, std::vector<int>(LOG));
            minEdge.assign(n, std::vector<int>(LOG, INT_MAX));

            // Заполнение для 2^0
            for (int v = 0; v < n; v++) {
                parent[v][0] = (v == root) ? root : par[v];
                minEdge[v][0] = (v == root) ? INT_MAX : parentWeight[v];
            }

            // Заполнение для 2^i
            for (int j = 1; j < LOG; j++) {
                for (int v = 0; v < n; v++) {
                    int mid = parent[v][j - 1];
                    parent[v][j] = parent[mid][j - 1];
                    minEdge[v][j] = std::min(minEdge[v][j - 1], minEdge[mid][j - 1]);
                }
            }
        }

        int query(int s, int t) const {
            if (s == t) return INT_MAX;
            if (s < 0 || s >= n || t < 0 || t >= n) return 0;

            int result = INT_MAX;
            int u = s;
            int v = t;

            // Выравниваем глубины
            if (depth[u] < depth[v]) {
                std::swap(u, v);
            }

            // Поднимаем u до глубины v
            int diff = depth[u] - depth[v];
            for (int j = 0; j < LOG; j++) {
                if (diff & (1 << j)) {
                    result = std::min(result, minEdge[u][j]);
                    u = parent[u][j];
                }
            }

            if (u == v) return result;

            // Поднимаем оба до LCA
            for (int j = LOG - 1; j >= 0; j--) {
                if (parent[u][j] != parent[v][j]) {
                    result = std::min(result, minEdge[u][j]);
                    result = std::min(result, minEdge[v][j]);
                    u = parent[u][j];
                    v = parent[v][j];
                }
            }

            // Последний шаг
            result = std::min(result, minEdge[u][0]);
            result = std::min(result, minEdge[v][0]);

            return result;
        }

        int getDepth(int v) const { return depth[v]; }
    };

    class BottleneckSolver {
    private:
        LCAPreprocessor lca;
        bool initialized;

    public:
        BottleneckSolver() : initialized(false) {}

        void build(const Graph& g) {
            if (g.n == 0) return;
            Graph mst = buildMaxST(g);
            lca.preprocess(mst, 0);
            initialized = true;
        }

        int query(int s, int t) const {
            if (!initialized) return 0;
            return lca.query(s, t);
        }

        std::vector<int> solve(const Graph& g,
            const std::vector<std::pair<int, int>>& pairs) {
            build(g);

            std::vector<int> answers;
            for (const auto& p : pairs) {
                answers.push_back(query(p.first, p.second));
            }
            return answers;
        }
    };
}

// ============================================
// 4. MSBPP ALGORITHM (Algorithm 7 from the paper)
// Исправленная версия с BFS вместо DFS
// ============================================

namespace MSBPP {

    Graph buildMaxST(const Graph& g) {
        if (g.n == 0) return Graph();

        Graph mst(g.n);
        std::vector<bool> inMST(g.n, false);
        using Element = std::tuple<int, int, int>;
        std::priority_queue<Element> pq;

        if (g.n == 0) return mst;

        // Находим стартовую вершину с рёбрами
        int startVertex = 0;
        for (int i = 0; i < g.n; i++) {
            if (!g.adj[i].empty()) {
                startVertex = i;
                break;
            }
        }

        inMST[startVertex] = true;

        for (const auto& edge : g.adj[startVertex]) {
            pq.push(std::make_tuple(edge.second, edge.first, startVertex));
        }

        while (!pq.empty()) {
            auto [weight, v, parent] = pq.top();
            pq.pop();

            if (inMST[v]) continue;

            inMST[v] = true;
            mst.addEdge(parent, v, weight);

            for (const auto& edge : g.adj[v]) {
                if (!inMST[edge.first]) {
                    pq.push(std::make_tuple(edge.second, edge.first, v));
                }
            }
        }

        return mst;
    }

    // BFS версия вместо DFS (избегаем переполнения стека)
    void bfsBottleneck(const Graph& tree, int source, std::vector<int>& bottleneck) {
        int n = tree.n;
        if (n == 0) return;

        std::queue<int> q;
        std::vector<bool> visited(n, false);

        bottleneck.assign(n, 0);
        bottleneck[source] = INT_MAX;
        visited[source] = true;
        q.push(source);

        while (!q.empty()) {
            int v = q.front();
            q.pop();

            for (const auto& edge : tree.adj[v]) {
                int to = edge.first;
                int weight = edge.second;

                if (!visited[to]) {
                    visited[to] = true;
                    bottleneck[to] = std::min(bottleneck[v], weight);
                    q.push(to);
                }
            }
        }
    }

    class MSSolver {
    private:
        Graph mst;
        bool initialized;

    public:
        MSSolver() : initialized(false) {}

        void build(const Graph& g) {
            if (g.n == 0) {
                mst = Graph();
                initialized = false;
                return;
            }
            mst = buildMaxST(g);
            initialized = true;
        }

        std::vector<int> solveFromSource(int s) const {
            std::vector<int> bottleneck;
            if (!initialized || mst.n == 0) {
                return bottleneck;
            }

            // Используем BFS вместо DFS
            bfsBottleneck(mst, s, bottleneck);
            return bottleneck;
        }

        std::vector<std::vector<int>> solve(const Graph& g,
            const std::vector<int>& sources) {
            build(g);

            std::vector<std::vector<int>> results;
            for (int s : sources) {
                results.push_back(solveFromSource(s));
            }
            return results;
        }
    };
}

#endif