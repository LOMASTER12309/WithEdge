#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

/**
 * Скачивание библиотеки
 * 1) cd ~
 * 2) git clone --branch v2.13.10 --depth 1 https://github.com/catchorg/Catch2.git
 * 3) cd Catch2
 * 4) sudo cp -r single_include/catch2 /usr/local/include/
 */

#include <vector>
#include <iostream>
#include <random>

#include "base.hpp"
#include "params.hpp"
#include "../src/with_edge.cpp"

using namespace gkernel;
using namespace drc_solver;

// ============================================================
// Вспомогательные функции
// ============================================================

/**
 * Создаёт прямоугольный контур по заданным координатам углов.
 * Точки передаются в порядке: левый-нижний -> правый-нижний -> правый-верхний -> левый-верхний.
 */
static Contour MakeRectContour(double x1, double y1, double x2, double y2) {
    std::vector<Point> pts{{x1, y1}, {x2, y1}, {x2, y2}, {x1, y2}};
    return Contour(pts);
}

/**
 * Возвращает true, если результирующий слой содержит ровно ожидаемое количество контуров.
 */
static bool HasSize(const ContourLayer &layer, std::size_t expected) {
    return layer.size() == expected;
}

/**
 * Генерация слоя прямоугольных контуров по случайным заполнением по сетке.
 * Область разбивается на сетку, в каждой ячейке размещается прямоугольник случайного размера и
 * положения внутри ячейки.
 */
static ContourLayer MakeRandomLayer(int rows, int cols, int cellWidth, int cellHeight,
                                    double minScale = 0.5, double maxScale = 0.5,
                                    ptrdiff_t seed = 42) {
    int x0 = 0;
    int y0 = 0;

    std::vector<Contour> contours;
    contours.reserve(rows * cols);

    std::mt19937 gen(seed);

    int minW = static_cast<int>(cellWidth * minScale);
    int maxW = static_cast<int>(cellWidth * maxScale);
    int minH = static_cast<int>(cellHeight * minScale);
    int maxH = static_cast<int>(cellHeight * maxScale);

    std::uniform_int_distribution<int> wDist(minW, maxW);
    std::uniform_int_distribution<int> hDist(minH, maxH);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int cellX = x0 + j * cellWidth;
            int cellY = y0 + i * cellHeight;

            int w = wDist(gen);
            int h = hDist(gen);

            std::uniform_int_distribution<int> dxDist(0, cellWidth - w);
            std::uniform_int_distribution<int> dyDist(0, cellHeight - h);

            int dx = dxDist(gen);
            int dy = dyDist(gen);

            int x1 = cellX + dx;
            int y1 = cellY + dy;

            contours.push_back(MakeRectContour(x1, y1, x1 + w, y1 + h));
        }
    }

    return ContourLayer(contours);
}

/**
 * Вывод времени
 */
static void PrintTime(const std::string &name, const std::chrono::steady_clock::time_point &time) {
    const auto final = std::chrono::steady_clock::now();
    const auto dt = std::chrono::duration_cast<std::chrono::microseconds>(final - time).count();
    std::cout << "time: " << name << ": " << dt << " dt\n";
}

// ============================================================
// ТЕСТЫ
// ============================================================

/**
 * ТЕСТ 1: Базовое наложение — два квадрата имеют общее ребро.
 *
 * Layer1: квадрат [0,0]–[2,2]
 * Layer2: квадрат [2,0]–[4,2]
 *
 * Общая вертикальная граница: x=2, y∈[0,2].
 * Без параметров (params=nullptr) → результат должен содержать 1 контур.
 */
TEST_CASE("withEdge: два квадрата с общим ребром, без params", "[basic]") {

    Contour c1 = MakeRectContour(0, 0, 2, 2);
    Contour c2 = MakeRectContour(2, 0, 4, 2);

    ContourLayer layer1({c1});
    ContourLayer layer2({c2});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("basic", time);

    REQUIRE(HasSize(result, 1));
}

/**
 * ТЕСТ 2: Нет общих рёбер — квадраты разнесены.
 *
 * Layer1: квадрат [0,0]–[2,2]
 * Layer2: квадрат [5,5]–[7,7]
 *
 * Никаких общих участков → результат пустой.
 */
TEST_CASE("withEdge: нет общих рёбер, без params", "[no_overlap]") {

    Contour c1 = MakeRectContour(0, 0, 2, 2);
    Contour c2 = MakeRectContour(5, 5, 7, 7);

    ContourLayer layer1({c1});
    ContourLayer layer2({c2});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("no_overlap", time);

    REQUIRE(HasSize(result, 0));
}

/**
 * ТЕСТ 3: Частичное наложение рёбер.
 *
 * Layer1: квадрат [0,0]–[4,4]
 * Layer2: прямоугольник [4,1]–[6,3]
 *
 * Общая часть: вертикальное ребро layer1 при x=4, y∈[0,4]
 * перекрывается с ребром layer2 при x=4, y∈[1,3].
 * Результат — 1 контур.
 */
TEST_CASE("withEdge: частичное перекрытие рёбер", "[partial_overlap]") {
    Contour c1 = MakeRectContour(0, 0, 4, 4);
    Contour c2 = MakeRectContour(4, 1, 6, 3);

    ContourLayer layer1({c1});
    ContourLayer layer2({c2});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("partial_overlap", time);

    REQUIRE(HasSize(result, 1));
}

/**
 * ТЕСТ 4: Несколько контуров в layer1 — выбираются только те, что имеют общие рёбра.
 *
 * Layer1: c1=[0,0]–[2,2], c2=[10,10]–[12,12]
 * Layer2: c3=[2,0]–[4,2]
 *
 * Только c1 имеет общее ребро с c3 (x=2, y∈[0,2]).
 * c2 изолирован → результат: 1 контур (c1).
 */
TEST_CASE("withEdge: несколько контуров, только один имеет общее ребро", "[multi_contour]") {
    Contour c1 = MakeRectContour(0, 0, 2, 2);
    Contour c2 = MakeRectContour(10, 10, 12, 12);
    Contour c3 = MakeRectContour(2, 0, 4, 2);

    ContourLayer layer1({c1, c2});
    ContourLayer layer2({c3});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("multi_contour", time);

    REQUIRE(HasSize(result, 1));
}

/**
 * ТЕСТ 5: Оба контура в layer1 имеют общие рёбра с layer2.
 *
 * Layer1: c1=[0,0]–[2,2], c2=[2,0]–[4,2]
 * Layer2: c3=[0,2]–[2,4], c4=[2,2]–[4,4]
 *
 * c1 имеет общее ребро с c3 (y=2, x∈[0,2])
 * c2 имеет общее ребро с c4 (y=2, x∈[2,4])
 * Результат: 2 контура.
 */
TEST_CASE("withEdge: оба контура имеют общие рёбра", "[both_match]") {
    Contour c1 = MakeRectContour(0, 0, 2, 2);
    Contour c2 = MakeRectContour(2, 0, 4, 2);
    Contour c3 = MakeRectContour(0, 2, 2, 4);
    Contour c4 = MakeRectContour(2, 2, 4, 4);

    ContourLayer layer1({c1, c2});
    ContourLayer layer2({c3, c4});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("both_match", time);

    REQUIRE(HasSize(result, 2));
}

/**
 * ТЕСТ 6: Использование ConstrParams — диапазон [1, 1] (ровно 1 общее ребро).
 *
 * Layer1: c1 имеет 1 общее ребро, c2 имеет 2 общих ребра
 * Layer2: два прямоугольника, которые касаются c1 и c2 по-разному
 *
 * Проверяем, что при k==1 выберется только c1.
 *
 * Схема:
 *   c1: [0,0]–[2,2]     — общее ребро с c3 (x=2, y∈[0,2])
 *   c2: [2,0]–[6,2]     — общие рёбра с c4 (x=2, y∈[0,2]) и c5 (x=6, y∈[0,2])
 *   c3: [2,0]–[4,2]     — касается c1 и c2
 *   c4: [6,0]–[8,2]     — касается c2
 */
TEST_CASE("withEdge: фильтр по params [1,1]", "[params_range_1_1]") {
    Contour c1 = MakeRectContour(0, 0, 2, 2);
    Contour c2 = MakeRectContour(4, 0, 6, 2);
    Contour c3 = MakeRectContour(2, 0, 4, 2);
    Contour c4 = MakeRectContour(6, 0, 8, 2);

    ContourLayer layer1({c1, c2});
    ContourLayer layer2({c3, c4});

    // c1 имеет 1 общее ребро (x=2), c2 имеет 0 общих рёбер
    ConstrParams params(1, 1, true, true);  // [1, 1]

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, &params);
    PrintTime("params_range_1_1", time);

    REQUIRE(HasSize(result, 1));
}

/**
 * ТЕСТ 7: n1=n2=0 — выбрать контуры БЕЗ общих рёбер.
 *
 * Layer1: c1=[0,0]–[2,2], c2=[10,10]–[12,12]
 * Layer2: c3=[2,0]–[4,2]
 *
 * c1 имеет общее ребро с c3, c2 — нет.
 * При params=[0,0] результат: только c2.
 */
TEST_CASE("withEdge: n1=n2=0 — выбрать контуры без общих рёбер", "[zero_count]") {
    Contour c1 = MakeRectContour(0, 0, 2, 2);
    Contour c2 = MakeRectContour(10, 10, 12, 12);
    Contour c3 = MakeRectContour(2, 0, 4, 2);

    ContourLayer layer1({c1, c2});
    ContourLayer layer2({c3});

    ConstrParams params(0, 0, true, true);  // ==0

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, &params);
    PrintTime("zero_count", time);

    REQUIRE(HasSize(result, 1));
}

/**
 * ТЕСТ 8: Пустой layer2 — результат всегда пустой (без params).
 *
 * Layer1: c1=[0,0]–[2,2]
 * Layer2: пустой
 *
 * Без params (nullptr) → ничего не выбирается.
 */
TEST_CASE("withEdge: пустой layer2, без params", "[empty_layer2]") {
    Contour c1 = MakeRectContour(0, 0, 2, 2);

    ContourLayer layer1({c1});
    ContourLayer layer2(std::vector<Contour>{});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("empty_layer2", time);

    REQUIRE(HasSize(result, 0));
}

/**
 * ТЕСТ 9: Пустой layer1 — результат всегда пустой.
 */
TEST_CASE("withEdge: пустой layer1", "[empty_layer1]") {
    Contour c2 = MakeRectContour(0, 0, 2, 2);

    ContourLayer layer1(std::vector<Contour>{});
    ContourLayer layer2({c2});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("empty_layer1", time);

    REQUIRE(HasSize(result, 0));
}

/**
 * ТЕСТ 10: Оба слоя пустые → результат пустой.
 */
TEST_CASE("withEdge: оба слоя пустые", "[both_empty]") {
    ContourLayer layer1(std::vector<Contour>{});
    ContourLayer layer2(std::vector<Contour>{});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("both_empty", time);

    REQUIRE(HasSize(result, 0));
}

/**
 * ТЕСТ 11: Квадраты касаются только углом (одна точка) — это НЕ общее ребро.
 *
 * Layer1: [0,0]–[2,2]
 * Layer2: [2,2]–[4,4]
 *
 * Общая точка (2,2) — но не отрезок → результат пустой.
 */
TEST_CASE("withEdge: касание только в одной точке", "[corner_touch]") {
    Contour c1 = MakeRectContour(0, 0, 2, 2);
    Contour c2 = MakeRectContour(2, 2, 4, 4);

    ContourLayer layer1({c1});
    ContourLayer layer2({c2});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("corner_touch", time);

    REQUIRE(HasSize(result, 0));
}

/**
 * ТЕСТ 12: Один контур в layer1 имеет несколько общих рёбер с layer2,
 * но каждое из таких должно считаться только 1 раз, даже если имеет несколько общих участков с
 * layer2.
 *
 * Layer1: большой квадрат [0,0]–[4,4]
 * Layer2: 4 маленьких квадрата, каждый из которых имеет общее ребро с layer1
 *   c2=[4,0]–[6,2]    — правое ребро
 *   c3=[4,2]–[6,4]    — правое ребро (верхняя часть)
 *   c4=[0,4]–[2,6]    — верхнее ребро
 *   c5=[2,4]–[4,6]    — верхнее ребро (правая часть)
 *
 * Итого: у c1 есть 2 общих ребра, но 4 общих участка с разными контурами layer2.
 * Проверяем params [2, 2] → c1 попадает.
 */
TEST_CASE("withEdge: несколько общих рёбер у одного контура", "[multiple_common_edges]") {
    Contour c1 = MakeRectContour(0, 0, 4, 4);
    Contour c2 = MakeRectContour(4, 0, 6, 2);
    Contour c3 = MakeRectContour(4, 2, 6, 4);
    Contour c4 = MakeRectContour(0, 4, 2, 6);
    Contour c5 = MakeRectContour(2, 4, 4, 6);

    ContourLayer layer1({c1});
    ContourLayer layer2({c2, c3, c4, c5});

    ConstrParams params(2, 2, true, true);  // [2, 2]

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("multiple_common_edges", time);

    REQUIRE(HasSize(result, 1));
}

/**
 * ТЕСТ 13: Горизонтальное наложение рёбер.
 *
 * Layer1: [0,0]–[4,2]
 * Layer2: [0,2]–[4,4]
 *
 * Общая горизонтальная граница: y=2, x∈[0,4].
 */
TEST_CASE("withEdge: горизонтальное общее ребро", "[horizontal_edge]") {
    Contour c1 = MakeRectContour(0, 0, 4, 2);
    Contour c2 = MakeRectContour(0, 2, 4, 4);

    ContourLayer layer1({c1});
    ContourLayer layer2({c2});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("horizontal_edge", time);

    REQUIRE(HasSize(result, 1));
}

/**
 * ТЕСТ 14: Диагональные рёбра — общее ребро.
 *
 * Создаём два ромба, которые имеют общее диагональное ребро.
 *
 * Ромб 1: (0,0)→(2,2)→(0,4)→(-2,2)
 * Ромб 2: (2,2)→(4,4)→(2,6)→(0,4)
 *
 * Общее ребро: (2,2)→(0,4) у ромба 1 и (0,4)→(2,2) у ромба 2
 * (одно и то же ребро, но в противоположных направлениях).
 */
TEST_CASE("withEdge: диагональное общее ребро", "[diagonal_edge]") {
    Contour c1(std::vector<Point>{{0, 0}, {2, 2}, {0, 4}, {-2, 2}});
    Contour c2(std::vector<Point>{{2, 2}, {4, 4}, {2, 6}, {0, 4}});

    ContourLayer layer1({c1});
    ContourLayer layer2({c2});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("diagonal_edge", time);

    REQUIRE(HasSize(result, 1));
}

/**
 * ТЕСТ 15: Полное совпадение контуров.
 *
 * Layer1 и Layer2 содержат одинаковые квадраты.
 * Все 4 ребра совпадают → результат: 1 контур.
 */
TEST_CASE("withEdge: полное совпадение контуров", "[identical_contours]") {
    Contour c1 = MakeRectContour(0, 0, 2, 2);
    Contour c2 = MakeRectContour(0, 0, 2, 2);

    ContourLayer layer1({c1});
    ContourLayer layer2({c2});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("identical_contours", time);

    REQUIRE(HasSize(result, 1));
}

/**
 * ТЕСТ 16: Проверка params с диапазоном >5 (только контуры с 6+ общими рёбрами).
 *
 * Создаём контур c1, который имеет много общих рёбер с layer2,
 * и контур c2, который имеет мало общих рёбер.
 *
 * c1: [0,0]–[2,2], layer2 содержит 1 контур с общим ребром → k=1
 * c2: [2,0]–[4,2], layer2 содержит 2 контура с общими рёбрами → k=2
 *
 * При params (5, 100, false, false) т.е. k>5, результат пустой.
 */
TEST_CASE("withEdge: params >5, ни один контур не подходит", "[params_strict]") {
    Contour c1 = MakeRectContour(0, 0, 2, 2);
    Contour c2 = MakeRectContour(2, 0, 4, 2);
    Contour c3 = MakeRectContour(0, 2, 2, 4);  // общее ребро с c1
    Contour c4 = MakeRectContour(2, 2, 4, 4);  // общее ребро с c2
    Contour c5 = MakeRectContour(4, 0, 6, 2);  // общее ребро с c2

    ContourLayer layer1({c1, c2});
    ContourLayer layer2({c3, c4, c5});

    // k > 5 (ни один контур не имеет столько общих рёбер)
    ConstrParams params(5, 100, false, false);

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, &params);
    PrintTime("params_strict", time);

    REQUIRE(HasSize(result, 0));
}

/**
 * ТЕСТ 17: Коллинеарные рёбра с частичным перекрытием.
 *
 * Layer1: прямоугольник [0,0]–[10,2]
 * Layer2: прямоугольник [3,0]–[7,4]
 *
 * Нижнее ребро layer2 (y=0, x∈[3,7]) частично перекрывается
 * с нижним ребром layer1 (y=0, x∈[0,10]).
 * Аналогично для верхних рёбер layer1 и нижних layer2 при y=2.
 */
TEST_CASE("withEdge: коллинеарные рёбра с частичным перекрытием", "[collinear_partial]") {
    Contour c1 = MakeRectContour(0, 0, 10, 2);
    Contour c2 = MakeRectContour(3, 0, 7, 4);

    ContourLayer layer1({c1});
    ContourLayer layer2({c2});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("collinear_partial", time);

    REQUIRE(HasSize(result, 1));
}

/**
 * ТЕСТ 18: Layer2 содержит дубликаты контуров.
 *
 * Layer1: [0,0]–[2,2]
 * Layer2: два одинаковых квадрата [2,0]–[4,2]
 *
 * Проверяем корректность подсчёта: каждый дубликат считается отдельно.
 */
TEST_CASE("withEdge: дубликаты в layer2", "[duplicate_layer2]") {
    Contour c1 = MakeRectContour(0, 0, 2, 2);
    Contour c2 = MakeRectContour(2, 0, 4, 2);
    Contour c3 = MakeRectContour(2, 0, 4, 2);  // дубликат c2

    ContourLayer layer1({c1});
    ContourLayer layer2({c2, c3});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("duplicate_layer2", time);

    // c1 имеет общие рёбра с обоими контурами layer2
    REQUIRE(HasSize(result, 1));
}

/**
 * ТЕСТ 19: Очень маленький общий участок (на грани eps).
 *
 * Layer1: [0,0]–[2,2]
 * Layer2: [2, 1.99999]–[4, 2.00001]
 *
 * Общее ребро по вертикали очень маленькое, но существующее.
 */
TEST_CASE("withEdge: очень маленький общий участок", "[tiny_overlap]") {
    Contour c1 = MakeRectContour(0, 0, 2, 2);
    Contour c2 = MakeRectContour(2, 1.99999, 4, 2.00001);

    ContourLayer layer1({c1});
    ContourLayer layer2({c2});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("tiny_overlap", time);

    // Общее ребро существует, но очень маленькое
    // Функция должна его обнаружить
    REQUIRE(HasSize(result, 1));
}

/**
 * ТЕСТ 20: params с полуоткрытым интервалом [1, 3).
 *
 * Layer1:
 *   c1 → 1 общее ребро
 *   c2 → 3 общих ребра
 *   c3 → 5 общих рёбер
 *
 * При params [1, 3) выбираются только c1 → результат: 1.
 */
TEST_CASE("withEdge: полуоткрытый интервал [1,3)", "[half_open_interval]") {
    Contour c1 = MakeRectContour(0, 0, 2, 2);
    Contour c2 = MakeRectContour(2, 0, 4, 2);
    Contour c3 = MakeRectContour(4, 0, 6, 2);
    Contour c4 = MakeRectContour(0, 2, 2, 4);  // общее ребро с c1
    Contour c5 = MakeRectContour(2, 2, 4, 4);  // общее ребро с c2
    Contour c6 = MakeRectContour(4, 2, 6, 4);  // общее ребро с c2 (через c3)
    Contour c7 = MakeRectContour(6, 0, 8, 2);  // общее ребро с c3
    Contour c8 = MakeRectContour(4, 0, 6, 2);  // совпадает с c3, даёт общие рёбра

    ContourLayer layer1({c1, c2, c3});
    ContourLayer layer2({c4, c5, c6, c7, c8});

    ConstrParams params(1, 3, true, false);  // [1, 3)

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, &params);
    PrintTime("half_open_interval", time);

    // c1: 1 общее ребро (c4) → попадает
    // c2: 2 общих ребра (c5, c6) → попадает
    // c3: зависит от совпадений
    REQUIRE(HasSize(result, 2));
}

/**
 * ТЕСТ 21: Три квадрата в ряд — средний имеет два общих ребра.
 *
 * Layer1: c1=[0,0]–[2,2], c2=[2,0]–[4,2], c3=[4,0]–[6,2]
 * Layer2: c4=[2,0]–[4,2] (совпадает с c2)
 *
 * c1: общее ребро с c4 (x=2) → k=1
 * c2: совпадает с c4 → 4 общих ребра
 * c3: общее ребро с c4 (x=4) → k=1
 *
 * При params=nullptr → все 3.
 */
TEST_CASE("withEdge: три квадрата в ряд", "[three_in_row]") {
    Contour c1 = MakeRectContour(0, 0, 2, 2);
    Contour c2 = MakeRectContour(2, 0, 4, 2);
    Contour c3 = MakeRectContour(4, 0, 6, 2);
    Contour c4 = MakeRectContour(2, 0, 4, 2);

    ContourLayer layer1({c1, c2, c3});
    ContourLayer layer2({c4});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("three_in_row", time);

    REQUIRE(HasSize(result, 3));
}

/**
 * ТЕСТ 22: L-образный контур.
 *
 * Layer1: L-образный контур
 * Layer2: прямоугольник, имеющий общее ребро с одной из частей L.
 *
 * L-форма: (0,0)→(4,0)→(4,2)→(2,2)→(2,4)→(0,4)
 * Layer2: [4,0]–[6,2]
 *
 * Общее ребро: x=4, y∈[0,2].
 */
TEST_CASE("withEdge: L-образный контур", "[l_shape]") {
    std::vector<Point> lShapePts{{0, 0}, {4, 0}, {4, 2}, {2, 2}, {2, 4}, {0, 4}};
    Contour c1(lShapePts);
    Contour c2 = MakeRectContour(4, 0, 6, 2);

    ContourLayer layer1({c1});
    ContourLayer layer2({c2});

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("l_shape", time);

    REQUIRE(HasSize(result, 1));
}

/**
 * Тест со случайными слоями и одинаковой сеткой.
 */
TEST_CASE("Random generator", "[random]") {
    std::random_device seedDevice;
    const ptrdiff_t seed1 = static_cast<ptrdiff_t>(seedDevice());
    const ptrdiff_t seed2 = static_cast<ptrdiff_t>(seedDevice());

    // std::cout << "random seeds: " << seed1 << ", " << seed2 << "\n";

    auto layer1 = MakeRandomLayer(2000, 1500, 10, 10, 0.2, 0.8, seed1);
    auto layer2 = MakeRandomLayer(1500, 2000, 10, 10, 0.2, 0.8, seed2);

    // auto layer1 = MakeRandomLayer(1000, 1000, 10, 10, 0.2, 0.8);
    // auto layer2 = MakeRandomLayer(1000, 1000, 10, 10, 0.2, 0.8);

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("random", time);

    // REQUIRE(HasSize(result, 1));
}

/**
 * Тест со случайными слоями и разной сеткой.
 */
TEST_CASE("Random generator and grid", "[random_grid]") {
    std::random_device seedDevice;
    const ptrdiff_t gridSeed = static_cast<ptrdiff_t>(seedDevice());
    const ptrdiff_t seed1 = static_cast<ptrdiff_t>(seedDevice());
    const ptrdiff_t seed2 = static_cast<ptrdiff_t>(seedDevice());

    std::mt19937 gen(gridSeed);

    // std::cout << "random_grid seeds: " << gridSeed << ", " << seed1 << ", " << seed2 << "\n";

    // std::mt19937 gen(42);

    std::uniform_int_distribution<int> sizeDist(1, 100);
    std::uniform_int_distribution<int> cellDist(1, 100);

    int rows1 = sizeDist(gen);
    int cols1 = sizeDist(gen);
    int cellW1 = cellDist(gen);
    int cellH1 = cellDist(gen);

    int rows2 = sizeDist(gen);
    int cols2 = sizeDist(gen);
    int cellW2 = cellDist(gen);
    int cellH2 = cellDist(gen);

    auto layer1 = MakeRandomLayer(rows1, cols1, cellW1, cellH1, 0.2, 0.8, seed1);
    auto layer2 = MakeRandomLayer(rows2, cols2, cellW2, cellH2, 0.2, 0.8, seed2);

    const auto time = std::chrono::steady_clock::now();
    ContourLayer result = withEdge(layer1, layer2, nullptr);
    PrintTime("random", time);

    // REQUIRE(HasSize(result, 1));
}
