import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import os

# Настройка стиля
plt.style.use('seaborn-v0_8-darkgrid')
plt.rcParams['font.size'] = 12
plt.rcParams['axes.titlesize'] = 14
plt.rcParams['axes.labelsize'] = 12
plt.rcParams['legend.fontsize'] = 11
plt.rcParams['figure.figsize'] = (10, 6)


def load_data():
    """Загрузка данных из CSV"""
    if os.path.exists('python_plot_data.csv'):
        df = pd.read_csv('python_plot_data.csv')
        print(f"Загружено {len(df)} строк из python_plot_data.csv")
        print(f"Диапазон n: {df['n'].min()} - {df['n'].max()}")
        return df
    elif os.path.exists('complete_graphs_results.csv'):
        df = pd.read_csv('complete_graphs_results.csv')
        print(f"Загружено {len(df)} строк из complete_graphs_results.csv")
        print(f"Диапазон n: {df['n'].min()} - {df['n'].max()}")
        return df
    else:
        print("Файл с данными не найден!")
        return None


def plot_baseline(df):
    """График для Бейзлайн: O(n²)"""
    fig, ax = plt.subplots()

    x = df['n'].values
    y = df['baseline_ms'].values

    # Теоретическая функция: O(n²)
    def theory(x, c):
        return c * x * x

    popt, _ = curve_fit(theory, x, y, maxfev=5000)

    # Построение
    ax.plot(x, y, 'd-', color='purple', linewidth=2, markersize=5,
            label='Бейзлайн (эксперимент)')
    ax.plot(x, theory(x, *popt), 'r--', linewidth=2,
            label=f'Теория O(n²), c={popt[0]:.8f}')

    ax.set_xlabel('Количество вершин (n)')
    ax.set_ylabel('Время выполнения (мс)')
    ax.set_title('Бейзлайн: время выполнения')
    ax.legend()
    ax.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig('01_baseline.png', dpi=150, bbox_inches='tight')
    plt.show()

    print(f"\nБейзлайн: O(n²), c = {popt[0]:.8f}")


def plot_offline(df):
    """График для Оффлайн: O(n² log n)"""
    fig, ax = plt.subplots()

    x = df['n'].values
    y = df['offline_ms'].values

    # Теоретическая функция: O(n² log n)
    def theory(x, c):
        return c * x * x * np.log2(x)

    popt, _ = curve_fit(theory, x, y, maxfev=5000)

    # Построение
    ax.plot(x, y, 'o-', color='blue', linewidth=2, markersize=5,
            label='Оффлайн (эксперимент)')
    ax.plot(x, theory(x, *popt), 'r--', linewidth=2,
            label=f'Теория O(n² log n), c={popt[0]:.10f}')

    ax.set_xlabel('Количество вершин (n)')
    ax.set_ylabel('Время выполнения (мс)')
    ax.set_title('Оффлайн: время выполнения')
    ax.legend()
    ax.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig('02_offline.png', dpi=150, bbox_inches='tight')
    plt.show()

    print(f"Оффлайн: O(n² log n), c = {popt[0]:.10f}")


def plot_online(df):
    """График для Онлайн: O(n²)"""
    fig, ax = plt.subplots()

    x = df['n'].values
    y = df['online_ms'].values

    # Теоретическая функция: O(n²)
    def theory(x, c):
        return c * x * x

    popt, _ = curve_fit(theory, x, y, maxfev=5000)

    # Построение
    ax.plot(x, y, 's-', color='green', linewidth=2, markersize=5,
            label='Онлайн (эксперимент)')
    ax.plot(x, theory(x, *popt), 'r--', linewidth=2,
            label=f'Теория O(n²), c={popt[0]:.8f}')

    ax.set_xlabel('Количество вершин (n)')
    ax.set_ylabel('Время выполнения (мс)')
    ax.set_title('Онлайн: время выполнения')
    ax.legend()
    ax.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig('03_online.png', dpi=150, bbox_inches='tight')
    plt.show()

    print(f"Онлайн: O(n²), c = {popt[0]:.8f}")


def main():
    print("=" * 60)
    print("ГЕНЕРАЦИЯ ГРАФИКОВ ДЛЯ АНАЛИЗА СЛОЖНОСТИ")
    print("=" * 60)

    df = load_data()
    if df is None:
        return

    print(f"\nДанные: {len(df)} измерений, n от {df['n'].min()} до {df['n'].max()}")

    print("\n" + "=" * 60)
    print("ПОСТРОЕНИЕ ГРАФИКОВ")
    print("=" * 60)

    plot_baseline(df)  # Бейзлайн
    plot_offline(df)  # Оффлайн
    plot_online(df)  # Онлайн

    print("\n" + "=" * 60)
    print("ГОТОВО!")
    print("=" * 60)
    print("\nСозданные файлы:")
    print("  01_baseline.png   - Бейзлайн (O(n²))")
    print("  02_offline.png    - Оффлайн (O(n² log n))")
    print("  03_online.png     - Онлайн (O(n²))")


if __name__ == "__main__":
    main()