#ifndef WIT_EDGE_LINE_HELPER_CPP
#define WIT_EDGE_LINE_HELPER_CPP

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include "base.hpp"
#include "params.hpp"

using namespace gkernel;
using namespace constants;
using namespace drc_solver;

// в params.hpp это чисто виртуальная функция без тела
// поэтому без её определение не выйдет
bool ConstrParams::init(std::string str) {
    return true;
}

constexpr double kNormalScale = 1e12;
constexpr std::int64_t kNormalScaleInt = static_cast<std::int64_t>(kNormalScale);

// Идентификатор прямой и ребра
struct LineId {
    LineId() noexcept {
    }

    LineId(std::int64_t _normalX, std::int64_t _normalY, double _signedDistance,
           double _leftInterval, double _rightInterval) noexcept
        : normalX(_normalX),
          normalY(_normalY),
          signedDistance(_signedDistance),
          leftInterval(_leftInterval),
          rightInterval(_rightInterval) {
    }

    // Квантованные координаты нормированной нормали из нуля к прямой
    std::int64_t normalX;
    std::int64_t normalY;

    // Расстояние до прямой
    double signedDistance;

    // Проекция ребра на прямую
    double leftInterval;
    double rightInterval;
};

// Данные о ребре: опорная прямая и индекс контура, которому принадлежит ребро
struct EdgeData {
    EdgeData() noexcept {
    }

    EdgeData(const LineId& _line, size_t _contourIndex) noexcept
        : line(_line), contourIndex(_contourIndex) {
    }

    LineId line;
    size_t contourIndex;
};

// Локальный масштаб точки, чтобы безопаснее считать скалярное произведение
// на больших координатах и не терять точность сильнее, чем нужно.
inline double LocalPointScale(const Point& firstPoint, const Point& secondPoint) noexcept {
    double pointScale = 1.0;
    pointScale = std::max(pointScale, std::abs(firstPoint.x));
    pointScale = std::max(pointScale, std::abs(firstPoint.y));
    pointScale = std::max(pointScale, std::abs(secondPoint.x));
    pointScale = std::max(pointScale, std::abs(secondPoint.y));
    return pointScale;
}

// Скалярное произведение считается через предварительное масштабирование координат
// Уменьшает риск лишних численных ошибок при больших значениях x/y
inline double ScaledDot(double vectorX, double vectorY, const Point& point,
                        double pointScale) noexcept {
    const double invPointScale = 1.0 / pointScale;
    const double scaledX = point.x * invPointScale;
    const double scaledY = point.y * invPointScale;
    return pointScale * std::fma(vectorX, scaledX, vectorY * scaledY);
}

inline std::int64_t QuantizeNormal(double value) noexcept {
    return static_cast<std::int64_t>(std::llround(value * kNormalScale));
}

// Построение идентификатора прямой и ребра
bool MakeLineId(const Edge& edge, LineId& out) noexcept {
    const Point& firstPoint = edge.min();
    const Point& secondPoint = edge.max();

    const double deltaX = secondPoint.x - firstPoint.x;
    const double deltaY = secondPoint.y - firstPoint.y;

    const double absDeltaX = std::abs(deltaX);
    const double absDeltaY = std::abs(deltaY);

    // Нормируем направление ребра в два шага:
    // сначала делим на max(|dx|, |dy|), чтобы избежать лишнего риска переполнения,
    // затем доводим до единичной длины через длину масштабированного вектора
    const double directionScale = std::max(absDeltaX, absDeltaY);

    if (directionScale < eps * 0.1) {
        return false;
    }

    // Частый случай - горизонтальная прямая
    if (deltaY == 0.0) {
        out = LineId{0, kNormalScaleInt, 0.0 + firstPoint.y, std::min(firstPoint.x, secondPoint.x),
                     std::max(firstPoint.x, secondPoint.x)};
        return true;
    }

    // Частый случай - вертикальная прямая
    if (deltaX == 0.0) {
        out = LineId{-kNormalScaleInt, 0, 0.0 - firstPoint.x, std::min(firstPoint.y, secondPoint.y),
                     std::max(firstPoint.y, secondPoint.y)};
        return true;
    }

    const double scaledDeltaX = deltaX / directionScale;
    const double scaledDeltaY = deltaY / directionScale;

    const double scaledDirectionLength =
        std::sqrt(scaledDeltaX * scaledDeltaX + scaledDeltaY * scaledDeltaY);

    double directionX = scaledDeltaX / scaledDirectionLength;
    double directionY = scaledDeltaY / scaledDirectionLength;

    // Фиксируем знак направления:
    // всегда выбираем представление с "неотрицательным" направлением
    // Это нужно, чтобы коллинеарные рёбра не получили противоположные нормали
    if (directionX < 0.0 || (directionX == 0.0 && directionY < 0.0)) {
        directionX = -directionX;
        directionY = -directionY;
    }

    // Единичная нормаль к прямой
    double normalX = -directionY;
    double normalY = directionX;

    double leftPoint;
    double rightPoint;

    // Для 1D-проекции выбираем более устойчивую ось:
    // если |normalY| больше (наклон меньше 45 градусов), удобнее работать по x, иначе по y
    if (std::abs(normalY) >= std::abs(normalX)) {
        leftPoint = firstPoint.x;
        rightPoint = secondPoint.x;
    } else {
        leftPoint = std::min(firstPoint.y, secondPoint.y);
        rightPoint = std::max(firstPoint.y, secondPoint.y);
    };

    // Расстояние до прямой считаем через масштабированное скалярное произведение
    const double pointScale = LocalPointScale(firstPoint, secondPoint);
    double signedDistance = ScaledDot(normalX, normalY, firstPoint, pointScale);

    out = LineId{QuantizeNormal(normalX), QuantizeNormal(normalY), signedDistance, leftPoint,
                 rightPoint};
    return true;
}

std::size_t CountLayerEdges(const ContourLayer& layer) noexcept {
    std::size_t edgeCount = 0;
    for (const Contour& contour : layer) {
        edgeCount += contour.size();
    }
    return edgeCount;
}

ContourLayer MakeContourLayer(std::vector<Contour>& contours) {
    ContourLayer resultLayer;
    resultLayer.addContours(contours);
    return resultLayer;
}

#endif
