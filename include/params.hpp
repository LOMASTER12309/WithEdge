#ifndef PARAMS_HPP
#define PARAMS_HPP

#include <string>
#include <vector>

namespace drc_solver {

struct Params {
    virtual bool init(std::string str) = 0;
};

/**
 * @struct ConstrParams
 * \brief Cтруктура для представления интервальных параметров
 */
struct ConstrParams : public Params {
    // первое (левое) значение
    double v1 = 0;

    // второе (правое) значение
    double v2 = 0;

    // вхождение v1 в отрезок
    bool inV1 = false;

    // вхождение v2 в отрезок
    bool inV2 = false;

    // внешний/внутренний интервал
    bool inside = false;

    ConstrParams() {}

    ConstrParams(double _v1, double _v2, bool _inV1, bool _inV2)
        : v1(_v1), v2(_v2), inV1(_inV1), inV2(_inV2) {}

    ConstrParams(double _v1, double _v2, bool _inV1, bool _inV2, bool _inside)
        : v1(_v1), v2(_v2), inV1(_inV1), inV2(_inV2), inside(_inside) {}

    // Примеры использования:
    //             V1  V2  inV1    inV2
    // <5.0        MIN 5.0 false   false
    // <=5.0       MIN 5.0 false   true
    // >5.0        5.0 MAX false   false
    // >=5.0       5.0 MAX true    false
    // >2.0 <5.0   2.0 5.0 false   false
    // >=2.0 <=5.0 2.0 5.0 true    true
    // ==5.0       5.0 5.0 true    true
    // !=5.0       5.0 5.0 false   false

    bool init(std::string str);
};

inline bool satisfy(double arg, const ConstrParams& cp) {
    if ((!cp.inV1 && !cp.inV2 && cp.v1 == cp.v2) && arg != cp.v1)
        return true;
    if ((!cp.inV1 && !cp.inV2) && ((arg - cp.v1) > 0 && (arg - cp.v2) < 0))
        return true;
    if ((cp.inV1 && !cp.inV2) && ((arg - cp.v1) >= 0 && (arg - cp.v2) < 0))
        return true;
    if ((!cp.inV1 && cp.inV2) && ((arg - cp.v1) > 0 && (arg - cp.v2) <= 0))
        return true;
    if ((cp.inV1 && cp.inV2) && ((arg - cp.v1) >= 0 && (arg - cp.v2) <= 0))
        return true;

    return false;
}

/**
 * @struct RectangleParams
 * \brief Параметры операции rectangle
 */
struct RectangleParams : public Params {
    std::vector<ConstrParams> cpv; // в нашем случае 1 или 2 интервала
    bool orth = false;             // ORTHOGONAL ONLY

    bool init(std::string str);
};

/**
 * @struct DensityParams
 * \brief Параметры операции density
 */
struct DensityParams : public Params {
    ConstrParams cpDensity; // выражение для плотности (>0.5) или (<=0.2)
    bool window = false;    // указано окно или нет (extent)
    int wx = 1;             // если окно указано, то это горизонтальный размер
    int wy = 1;             // если окно указано, то это вертикальный размер
    bool print = false;     // вывод результатов в файл
    std::string fileName;   // имя файла
    bool step =
        false;  // указаны размеры она сканирования или нет. Если не указаны - совпадают с wx и wy
    int sx = 1; // длина окна сканирования
    int sy = 1; // высота окна сканирования

    DensityParams() {}

    DensityParams(ConstrParams cp, bool _window, int _wx, int _wy)
        : cpDensity(cp), window(_window), wx(_wx), wy(_wy) {}

    bool init(std::string str);
};

/**
 * @enum SelectionType
 * \brief Тип для отбора ребер в операциях Ext, Int, Enc
 */
enum SelectionType {
    NonIntersect, // непересекающиеся
    Singular, // необходимость вычисления расстояния между ребрами входных слоев в случае их касания
              // в одной точке
    AbutOnly, // необходимость вычисления расстояния между пересекающимися ребрами входных слоев
    AbutAngle, // ограничение на угол пересечения ребер входных слоев в точке пересечения рёбер при
               // использовании abut
    AbutSingular // комбинация
};

/**
 * @enum MetricsMeasurement
 * \brief Тип области расширения (сужения) для каждого ребра, состоящую из полуплоскости всех точек,
 * которые находятся в пределах заданного расстояния от ребра. Для операций Ext, Int, Enc
 */
enum MetricsMeasurement {
    Square = 0, // область с прямоугольными границами, выходящими за конечные точки выбранных ребер
    Euclidean = 1, // область с границами в четверть окружности, которые выходят за конечные точки
                   // выбранных ребер
    Opposite = 2,  // область с прямоугольными границами, которые не выходят за конечные точки
                   // выбранных ребер
    OppositeExtended = 3, // область с прямоугольными границами, которые могут выходить за углы
                          // выбранных ребер в зависимости от заданного значения
    OppositeSymmetric = 4 // противоположная (Opposite) метрика для измерения с последующей
                          // обработкой для использования с непараллельными рёбрами
};

/**
 * @struct MeasurementParams
 * \brief Параметры измерения в функциях Ext, Int, Enc
 */
struct MeasurementParams : public Params {
    MetricsMeasurement typeMetricsMeasurement; // тип области расширения
    SelectionType selType;                     // режим отбора рёбер
    ConstrParams cp;                           // заданный интервал разбиения
    double angle; // допустимое ограничение угла между измеряемыми рёбрами

    bool init(std::string str);
};

/**
 * @struct ExpandParams
 * \brief Параметры операции expand edges
 */
struct ExpandParams : public Params {
    bool inside;         // true, false
    double insideValue;  // величина расширения
    bool outside;        // true, false
    double outsideValue; // величина расширения
    bool extend;         // увеличение ребер с концов
    double extendValue;  // величина увеличения длины ребер с каждого конца
    bool cornerFill;     // заполнение "углов"

    bool init(std::string str);
};

/**
 * @struct SizeParams
 * \brief Параметры операции size
 */
struct SizeParams : public Params {
    double sizeValue = 0;
    bool underover = false;

    SizeParams() {}

    SizeParams(double _sizeValue) : sizeValue(_sizeValue) {}

    SizeParams(double _sizeValue, bool _underover) : sizeValue(_sizeValue), underover(_underover) {}

    bool init(std::string str);
};

} // namespace drc_solver

#endif
