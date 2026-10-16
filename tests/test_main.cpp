#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>
#include <cstdint>
#include <bit>
#include <iomanip>

#include "base.hpp"
#include "../src/line_helper.cpp"

using namespace gkernel;

constexpr double EPS = 1e-12;

bool projectionsEqual(const LineId& p1, const LineId& p2) {
    return (std::abs(p1.normalX - p2.normalX) < EPS) &&
           (std::abs(p1.normalY - p2.normalY) < EPS) &&
           (std::abs(p1.signedDistance - p2.signedDistance) < EPS);
}

void printProjection(const std::string& label, const LineId& p, int preprecision) {
    std::cout << label << ": normalX=" << std::fixed << std::setprecision(preprecision) << p.normalX
              << ", normalY=" << p.normalY << ", signedDistance=" << p.signedDistance << "\n";
}

LineId MakeProj(const gkernel::Edge& edge) {
    LineId lineId;
    MakeLineId(edge, lineId);
    return lineId;
}

int main() {
    std::cout << "=== Тест функции MakeLineId ===\n\n";

    // Тест 1: Два ребра на одной горизонтальной прямой
    std::cout << "Тест 1: Рёбра на одной горизонтальной прямой (y=0)\n";
    Edge edge1(Point(0, 0), Point(2, 0));
    Edge edge2(Point(3, 0), Point(5, 0));

    LineId proj1 = MakeProj(edge1);
    LineId proj2 = MakeProj(edge2);

    printProjection("Ребро 1 (0,0)->(2,0)", proj1, 8);
    printProjection("Ребро 2 (3,0)->(5,0)", proj2, 8);

    if (projectionsEqual(proj1, proj2)) {
        std::cout << "УСПЕХ: Тройки (normalX, normalY, signedDistance) одинаковы\n\n";
    } else {
        std::cout << "ОШИБКА: Тройки различаются!\n\n";
    }

    // Тест 2: Два ребра на одной вертикальной прямой
    std::cout << "Тест 2: Рёбра на одной вертикальной прямой (x=1)\n";
    Edge edge3(Point(1, 0), Point(1, 2));
    Edge edge4(Point(1, 3), Point(1, 5));

    LineId proj3 = MakeProj(edge3);
    LineId proj4 = MakeProj(edge4);

    printProjection("Ребро 3 (1,0)->(1,2)", proj3, 8);
    printProjection("Ребро 4 (1,3)->(1,5)", proj4, 8);

    if (projectionsEqual(proj3, proj4)) {
        std::cout << "УСПЕХ: Тройки (normalX, normalY, signedDistance) одинаковы\n\n";
    } else {
        std::cout << "ОШИБКА: Тройки различаются!\n\n";
    }

    // Тест 3: Рёбра с противоположным направлением на одной прямой
    std::cout << "Тест 3: Рёбра с противоположным направлением на одной прямой\n";
    Edge edge5(Point(0, 0), Point(2, 0));  // вправо
    Edge edge6(Point(5, 0), Point(3, 0));  // влево

    LineId proj5 = MakeProj(edge5);
    LineId proj6 = MakeProj(edge6);

    printProjection("Ребро 5 (0,0)->(2,0)", proj5, 8);
    printProjection("Ребро 6 (5,0)->(3,0)", proj6, 8);

    if (projectionsEqual(proj5, proj6)) {
        std::cout << "УСПЕХ: Направление не влияет на опорную прямую\n\n";
    } else {
        std::cout << "ОШИБКА: Тройки различаются!\n\n";
    }

    // Тест 4: Наклонные рёбра на одной прямой (диагональ y=x)
    std::cout << "Тест 4: Наклонные рёбра на одной прямой (y=x)\n";
    Edge edge7(Point(0, 0), Point(3, 3));
    Edge edge8(Point(4, 4), Point(6, 6));

    LineId proj7 = MakeProj(edge7);
    LineId proj8 = MakeProj(edge8);

    printProjection("Ребро 7 (0,0)->(3,3)", proj7, 8);
    printProjection("Ребро 8 (4,4)->(6,6)", proj8, 8);

    if (projectionsEqual(proj7, proj8)) {
        std::cout << "УСПЕХ: Тройки (normalX, normalY, signedDistance) одинаковы\n\n";
    } else {
        std::cout << "ОШИБКА: Тройки различаются!\n\n";
    }

    // Тест 5: Рёбра на разных параллельных прямых
    std::cout << "Тест 5: Рёбра на разных параллельных прямых (y=0 и y=1)\n";
    Edge edge9(Point(0, 0), Point(2, 0));
    Edge edge10(Point(0, 1), Point(2, 1));

    LineId proj9 = MakeProj(edge9);
    LineId proj10 = MakeProj(edge10);

    printProjection("Ребро 9 (0,0)->(2,0)", proj9, 8);
    printProjection("Ребро 10 (0,1)->(2,1)", proj10, 8);

    if (!projectionsEqual(proj9, proj10)) {
        std::cout << "УСПЕХ: signedDistance различается для параллельных прямых\n\n";
    } else {
        std::cout << "ОШИБКА: Тройки одинаковы, но должны различаться!\n\n";
    }

    // Тест 6: Маленький угол между прямыми
    std::cout << "Тест 6: Близкий угол прямых\n";
    Edge edge11(Point(0, 0), Point(0, 1));
    Edge edge12(Point(-1, -1000000000000), Point(1, 1000000000000));

    LineId proj11 = MakeProj(edge11);
    LineId proj12 = MakeProj(edge12);

    printProjection("Ребро 11 ", proj11, 15);
    printProjection("Ребро 12 ", proj12, 15);

    if (!projectionsEqual(proj11, proj12)) {
        std::cout << "УСПЕХ: Нормали разные\n\n";
    } else {
        std::cout << "ОШИБКА: Тройки одинаковы, но должны различаться!\n\n";
    }
    return 0;
}
