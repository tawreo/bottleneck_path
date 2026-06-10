#include "algorithms.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>
#include <string>

using namespace std;

template<typename Func>
double measureTime(Func func) {
    auto start = chrono::high_resolution_clock::now();
    func();
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration<double, milli>(end - start).count();
}

// Генерация полносвязного графа
Graph generateCompleteGraph(int vertices, unsigned seed = 42) {
    Graph g(vertices);
    mt19937 rng(seed);
    uniform_int_distribution<int> weightDist(1, 100);

    for (int i = 0; i < vertices; i++) {
        for (int j = i + 1; j < vertices; j++) {
            g.addEdge(i, j, weightDist(rng));
        }
    }
    return g;
}

int main() {
    srand((unsigned int)time(nullptr));

    cout << "============================================" << endl;
    cout << "COMPLEXITY ANALYSIS - COMPLETE GRAPHS" << endl;
    cout << "Based on the paper by Kaymakov & Malyshev" << endl;
    cout << "============================================" << endl;

    // Параметры тестирования: n от 10 до 500 с шагом 10
    vector<int> graphSizes;
    for (int i = 10; i <= 500; i += 10) {
        graphSizes.push_back(i);
    }

    int queriesPerSize = 200;   // Количество запросов
    int runsPerSize = 3;        // Количество запусков для усреднения

    cout << "\nConfiguration:" << endl;
    cout << "  Graph type: Complete Graph K_n" << endl;
    cout << "  n range: 10 to 500 (step 10)" << endl;
    cout << "  Total configurations: " << graphSizes.size() << endl;
    cout << "  For n=500, edges = " << 500 * 499 / 2 << " (~124,750 edges)" << endl;
    cout << "  Queries per size: " << queriesPerSize << endl;
    cout << "  Runs per size: " << runsPerSize << endl;
    cout << string(80, '-') << endl;

    // Оценка времени выполнения
    cout << "\nEstimated total time: ~1-1.5 hours" << endl;
    cout << "Press Enter to continue or Ctrl+C to cancel...";
    cin.get();

    cout << "\nStarting tests...\n" << endl;
    cout.flush();

    struct Result {
        int n, m;
        double baseline, offline, online;
        double baseline_std, offline_std, online_std;
    };
    vector<Result> results;

    for (size_t idx = 0; idx < graphSizes.size(); idx++) {
        int n = graphSizes[idx];
        int m = n * (n - 1) / 2;

        vector<double> baseline_times, offline_times, online_times;

        // Прогресс
        double progress = (double)idx / graphSizes.size() * 100;

        cout << "  [" << fixed << setprecision(1) << progress << "%] n=" << setw(4) << n
            << " (edges=" << m << ") [" << flush;

        for (int run = 0; run < runsPerSize; run++) {
            cout << "." << flush;

            // Генерация полного графа
            Graph g = generateCompleteGraph(n, 12345 + n * 100 + run);

            // Генерация запросов
            vector<pair<int, int>> queries;
            for (int i = 0; i < queriesPerSize; i++) {
                int s = rand() % n;
                int t = rand() % n;
                if (s == t) t = (t + 1) % n;
                queries.push_back({ s, t });
            }

            // ============================================
            // 1. БЕЙЗЛАЙН (Baseline) алгоритм - Camerini
            // ============================================
            double t = measureTime([&]() {
                Baseline::solve(g, queries);
                });
            baseline_times.push_back(t);

            // ============================================
            // 2. ОФФЛАЙН (Offline) алгоритм
            // ============================================
            t = measureTime([&]() {
                Offline::solve(g, queries);
                });
            offline_times.push_back(t);

            // ============================================
            // 3. ОНЛАЙН (Online) алгоритм
            // ============================================
            t = measureTime([&]() {
                Online::BottleneckSolver solver;
                solver.solve(g, queries);
                });
            online_times.push_back(t);

            // Предупреждение для больших n
            if (n >= 400 && run == 0) {
                cout << "\n  [INFO] n=" << n << " (baseline may take ~"
                    << (int)(0.015 * n * n / 1000) << " sec per query)" << flush;
            }
        }

        cout << "] " << flush;

        // Вычисляем средние и стандартные отклонения
        double baseline_mean = 0, offline_mean = 0, online_mean = 0;
        double baseline_std = 0, offline_std = 0, online_std = 0;

        if (runsPerSize > 0) {
            baseline_mean = accumulate(baseline_times.begin(), baseline_times.end(), 0.0) / runsPerSize;
            offline_mean = accumulate(offline_times.begin(), offline_times.end(), 0.0) / runsPerSize;
            online_mean = accumulate(online_times.begin(), online_times.end(), 0.0) / runsPerSize;

            if (runsPerSize > 1) {
                double baseline_var = 0, offline_var = 0, online_var = 0;
                for (double t : baseline_times) baseline_var += (t - baseline_mean) * (t - baseline_mean);
                for (double t : offline_times) offline_var += (t - offline_mean) * (t - offline_mean);
                for (double t : online_times) online_var += (t - online_mean) * (t - online_mean);

                baseline_std = sqrt(baseline_var / (runsPerSize - 1));
                offline_std = sqrt(offline_var / (runsPerSize - 1));
                online_std = sqrt(online_var / (runsPerSize - 1));
            }
        }

        Result res;
        res.n = n;
        res.m = m;
        res.baseline = baseline_mean;
        res.offline = offline_mean;
        res.online = online_mean;
        res.baseline_std = baseline_std;
        res.offline_std = offline_std;
        res.online_std = online_std;
        results.push_back(res);

        cout << "base=" << fixed << setprecision(2) << res.baseline
            << " ±" << res.baseline_std
            << " off=" << res.offline
            << " ±" << res.offline_std
            << " on=" << res.online
            << " ±" << res.online_std << "ms" << endl;
        cout.flush();
    }

    // ============================================
    // ВЫВОД ТАБЛИЦЫ РЕЗУЛЬТАТОВ
    // ============================================
    cout << "\n" << string(140, '=') << endl;
    cout << "RESULTS FOR COMPLETE GRAPHS K_n (n=10..500, step=10)" << endl;
    cout << string(140, '=') << endl;

    cout << "\n" << setw(8) << "n"
        << setw(12) << "m"
        << setw(16) << "Бейзлайн(ms)"
        << setw(16) << "Оффлайн(ms)"
        << setw(16) << "Онлайн(ms)"
        << setw(16) << "n*log2(n)"
        << setw(16) << "n^2"
        << setw(16) << "n√n" << endl;
    cout << string(130, '-') << endl;

    for (const auto& r : results) {
        double nlogn = r.n * log2(r.n);
        double n2 = (double)r.n * r.n;
        double nsqrtn = r.n * sqrt(r.n);

        cout << setw(8) << r.n
            << setw(12) << r.m
            << setw(16) << fixed << setprecision(3) << r.baseline
            << setw(16) << fixed << setprecision(3) << r.offline
            << setw(16) << fixed << setprecision(3) << r.online
            << setw(16) << fixed << setprecision(0) << nlogn
            << setw(16) << fixed << setprecision(0) << n2
            << setw(16) << fixed << setprecision(0) << nsqrtn << endl;
    }

    // ============================================
    // АНАЛИЗ СЛОЖНОСТИ
    // ============================================
    if (results.size() >= 2) {
        cout << "\n" << string(140, '=') << endl;
        cout << "COMPLEXITY ANALYSIS" << endl;
        cout << string(140, '=') << endl;

        int n_first = results.front().n;
        int n_last = results.back().n;

        double baseline_growth = results.back().baseline / results.front().baseline;
        double offline_growth = results.back().offline / results.front().offline;
        double online_growth = results.back().online / results.front().online;

        double nlogn_growth = (n_last * log2(n_last)) / (n_first * log2(n_first));
        double n2_growth = (double)(n_last * n_last) / (n_first * n_first);
        double nsqrtn_growth = (n_last * sqrt(n_last)) / (n_first * sqrt(n_first));

        cout << "\nGrowth from n=" << n_first << " to n=" << n_last << ":" << endl;
        cout << "  Бейзлайн time growth: " << fixed << setprecision(2) << baseline_growth << "x" << endl;
        cout << "  Оффлайн time growth:  " << fixed << setprecision(2) << offline_growth << "x" << endl;
        cout << "  Онлайн time growth:   " << fixed << setprecision(2) << online_growth << "x" << endl;
        cout << "  O(n log n) growth:    " << fixed << setprecision(2) << nlogn_growth << "x" << endl;
        cout << "  O(n²) growth:         " << fixed << setprecision(2) << n2_growth << "x" << endl;
        cout << "  O(n√n) growth:        " << fixed << setprecision(2) << nsqrtn_growth << "x" << endl;

        // Вывод заключения
        cout << "\nConclusion:" << endl;

        double baseline_ratio_n2 = baseline_growth / n2_growth;
        double offline_ratio_n2_logn = offline_growth / n2_growth;
        double online_ratio_n2 = online_growth / n2_growth;

        cout << "  Бейзлайн: O(k * m) ≈ O(n²) - ratio to O(n²) = " << baseline_ratio_n2 << endl;
        cout << "  Оффлайн:  O((m+k) log n) ≈ O(n² log n) - ratio to O(n²) = " << offline_ratio_n2_logn << endl;
        cout << "  Онлайн:   O(m + (n+k) log n) ≈ O(n²) - ratio to O(n²) = " << online_ratio_n2 << endl;

        if (baseline_ratio_n2 > 0.5 && baseline_ratio_n2 < 2.0) {
            cout << "\n  [+] Бейзлайн: подтверждает O(n²)" << endl;
        }
        if (online_ratio_n2 > 0.5 && online_ratio_n2 < 2.0) {
            cout << "  [+] Онлайн: подтверждает O(n²)" << endl;
        }
    }

    // ============================================
    // СОХРАНЕНИЕ В CSV
    // ============================================
    ofstream csv("complete_graphs_results.csv");
    csv << "n,m,baseline_ms,baseline_std,offline_ms,offline_std,online_ms,online_std,nlogn,n2,nsqrtn\n";
    for (const auto& r : results) {
        double nlogn = r.n * log2(r.n);
        double n2 = (double)r.n * r.n;
        double nsqrtn = r.n * sqrt(r.n);
        csv << r.n << "," << r.m << ","
            << r.baseline << "," << r.baseline_std << ","
            << r.offline << "," << r.offline_std << ","
            << r.online << "," << r.online_std << ","
            << nlogn << "," << n2 << "," << nsqrtn << "\n";
    }
    csv.close();

    // Сохраняем упрощённый формат для Python
    ofstream py_csv("python_plot_data.csv");
    py_csv << "n,baseline_ms,offline_ms,online_ms,nlogn,n2,nsqrtn\n";
    for (const auto& r : results) {
        double nlogn = r.n * log2(r.n);
        double n2 = (double)r.n * r.n;
        double nsqrtn = r.n * sqrt(r.n);
        py_csv << r.n << "," << r.baseline << "," << r.offline << "," << r.online << ","
            << nlogn << "," << n2 << "," << nsqrtn << "\n";
    }
    py_csv.close();

    cout << "\n" << string(80, '=') << endl;
    cout << "SUMMARY" << endl;
    cout << string(80, '=') << endl;
    cout << "\n[OK] Results saved to:" << endl;
    cout << "  - complete_graphs_results.csv" << endl;
    cout << "  - python_plot_data.csv" << endl;
    cout << "\nTotal measurements: " << results.size() << " configurations" << endl;
    cout << "n range: 10 to 500 (step 10)" << endl;
    cout << "\nAlgorithms tested:" << endl;
    cout << "  - Бейзлайн (Baseline/Camerini) - O(k * n²)" << endl;
    cout << "  - Оффлайн (Offline MPBPP) - O((m+k) log n)" << endl;
    cout << "  - Онлайн (Online MPBPP) - O(m + (n+k) log n)" << endl;

    cout << "\n============================================" << endl;
    cout << "TESTING COMPLETED" << endl;
    cout << "============================================" << endl;

    return 0;
}