#ifndef __EDGEMEASUREMENT_HPP
#define __EDGEMEASUREMENT_HPP
#include <functional>
#include <valarray>
#include <vector>

#include "base.hpp"
#include "params.hpp"

using namespace gkernel;
using data_type = double;

namespace measurement {
using segmentExt_id = size_t;

/**
 * @enum TypeMeasurement
 * \brief Тип измерения { Ext, Int, Enc }
 */
enum TypesMeasurement { InternalEdges = 0, ExternalEdges = 1, EnclosureEdges = 2 };

class EdgeMeasurement {
public:
    constexpr static double EPS = 1e-9;

    struct PropertiesCircleSector {
        double xc;
        double yc;
        double r;
        bool sectorX;
        bool sectorY;
    };

    // геометрические характеристики области расширения
    struct PropertiesArea {
        double p1x, p1y;   // вершина 1 нижней границы расширения
        double p2x, p2y;   // вершина 2 нижней границы расширения
        bool increaseSegX; // направление по X (возрастание) нижней границы расширения
        bool increaseSegY; // направление по Y (возрастание) нижней границы расширения
        double u1x, u1y;   // вершина 1 верхней границы расширения
        double u2x, u2y;   // вершина 2 верхней границы расширения

        // коэфициенты прямых, ограничивающих область расширения
        double
            kSeg[4]; // очередность 0 - Down (исходный отрезок), 1 - Up, 2 - SideLeft, 3 - SideRight
        double mSeg[4];
        bool isVerticalSeg[4];
        double pX1[4];
        double pY1[4];
        double pX2[4];
        double pY2[4];
        bool sideSectorCircle[4]; // флаги указывающщие, что граница области расширения является
                                  // секитор окружности в соотвествующей стороне
        PropertiesCircleSector prCircleSector[4]; // параметры сектора окружности в случае его
                                                  // наличия на соответствующей стороне границы
        size_t indexSeg;                          // индекс исходного ребра в массиве kSeg
        PropertiesArea();
    };

    // геометрические характеристики дискредитироваанной упрощенной области в вертикальной боковой
    // границей либо отсутствием её
    struct PropertiesSimpleDiscreteArea {
        double p1x, p1y; // вершина 1 нижней границы расширения
        double p2x, p2y; // вершина 2 нижней границы расширения
        double u1x, u1y; // вершина 1 верхней границы расширения
        double u2x, u2y; // вершина 2 верхней границы расширения
        std::vector<double>* discreteXSeg;
        std::vector<double>* discreteYSeg1;
        std::vector<double>* discreteYSeg2;
        bool flagIntersectUp;
        bool flagIntersectDown;

        PropertiesSimpleDiscreteArea(std::vector<double>& _discreteXSeg,
                                     std::vector<double>& _discreteYSeg1,
                                     std::vector<double>& _discreteYSeg2,
                                     double _p1x,
                                     double _p1y,
                                     double _p2x,
                                     double _p2y,
                                     double _u1x,
                                     double _u1y,
                                     double _u2x,
                                     double _u2y,
                                     bool _flagIntersectDown,
                                     bool _flagIntersectUp) {
            p1x = _p1x;
            p1y = _p1y;
            p2x = _p2x;
            p2y = _p2y;
            u1x = _u1x;
            u1y = _u1y;
            u2x = _u2x;
            u2y = _u2y;
            discreteXSeg = &_discreteXSeg;
            discreteYSeg1 = &_discreteYSeg1;
            discreteYSeg2 = &_discreteYSeg2;
            flagIntersectUp = _flagIntersectUp;
            flagIntersectDown = _flagIntersectDown;
        }
    };

    // структура обертка сегмента
    struct SegmentExt {
        SegmentExt();
        SegmentExt(const Edge* _seg,
                   std::size_t _idSeg,
                   std::size_t _idContour,
                   std::size_t _idParentSeg,
                   std::size_t _id);
        SegmentExt(const Edge* _seg,
                   const Point& begin_point,
                   const Point& end_point,
                   std::size_t _idSeg,
                   std::size_t _idContour,
                   std::size_t _idParentSeg,
                   std::size_t _id);

        const Point& Start() const {
            return begin_point;
        }

        const Point& End() const {
            return end_point;
        }

        const Point& Min() const {
            if (directionMinMax)
                return begin_point;
            else
                return end_point;
        }

        const Point& Max() const {
            if (directionMinMax)
                return end_point;
            else
                return begin_point;
        }

        segmentExt_id GetExtId() const {
            return id;
        }

        void SetExtId(segmentExt_id idElem) {
            id = idElem;
        }

        segmentExt_id GetId() const {
            return idSeg;
        }

        segmentExt_id GetIdParent() const {
            return idParentSeg;
        }

        std::size_t GetIdContour() const {
            return idContour;
        }

        const Edge* GetSeg() const {
            return seg;
        }

        double GetK() const {
            return k;
        }

        double GetM() const {
            return m;
        }

        bool IsVertical() const {
            return isVertical;
        }

        bool GetCoDirectionParent() const {
            return coDirectionParent;
        }

        double GetY(double x, double eps) const;
        double GetY(double x) const;

    private:
        const Edge* seg;    // исходное ребро
        size_t idSeg;       // индекс исходного ребра в контейнере рёбер
        size_t idContour;   // индекс контура
        size_t idParentSeg; // индекс усеченного ребра в контейнере усеченных рёбер (после взаимного
                            // прорезания слоев)
        segmentExt_id id;   // индекс текущего ребра в контейнере сегментов-обёрток
        bool isVertical;
        bool coDirectionParent;
        bool directionMinMax;
        double k;
        double m;
        Point begin_point;
        Point end_point;
        double EPSP = 1e-9;
    };

    // структура хранящая часть сегмента
    struct SegmentPart {
        const SegmentExt* seg;
        double startT;
        double endT;
        double startX;
        double startY;
        double endX;
        double endY;
        bool cut;

        SegmentPart(SegmentExt* _seg) {
            startX = _seg->Start().x;
            startY = _seg->Start().y;
            endX = _seg->End().x;
            endY = _seg->End().y;
            seg = _seg;
            startT = 0;
            endT = 1;
            cut = false;
        }

        SegmentPart(const SegmentExt* _seg) {
            startX = _seg->Start().x;
            startY = _seg->Start().y;
            endX = _seg->End().x;
            endY = _seg->End().y;
            seg = _seg;
            startT = 0;
            endT = 1;
            cut = false;
        }

        SegmentPart(SegmentExt* _seg, gkernel::Point& _start, gkernel::Point& _end) {
            seg = _seg;
            startX = _start.x;
            startY = _start.y;
            endX = _end.x;
            endY = _end.y;
            cut = IsCut();
            startT = GetTPointSegment(startX, startY, seg);
            endT = GetTPointSegment(endX, endY, seg);
        }

        bool IsCut() {
            if ((fabs(seg->Start().x - startX) > EPS) || (fabs(seg->Start().y - startY) > EPS) ||
                (fabs(seg->End().x - endX) > EPS) || (fabs(seg->End().y - endY) > EPS))
                return true;
            return false;
        }

        SegmentPart(SegmentExt* _seg, double _startX, double _startY, double _endX, double _endY) {
            seg = _seg;
            startX = _startX;
            startY = _startY;
            endX = _endX;
            endY = _endY;
            cut = IsCut();
            startT = GetTPointSegment(startX, startY, seg);
            endT = GetTPointSegment(endX, endY, seg);
        }

        void SetStartPoint(double _startX, double _startY) {
            startX = _startX;
            startY = _startY;
            cut = IsCut();
        }

        void SetEndPoint(double _endX, double _endY) {
            endX = _endX;
            endY = _endY;
            cut = IsCut();
        }

        double GetTPointSegment(double pointSegX, double pointSegY, const SegmentExt* seg);
        void GetPointXY(double t, double& x, double& y);
        gkernel::Point GetPoint(double t);
    };

    enum Segment_status {
        verticalUp = -1,
        intersection = 0,
        start = 1,
        vertical = 2,
        end = 3,
        verticalDown = 5
    };

    struct SegmentsPoint {
        size_t idSeg;
        double value;
        Segment_status segStatus;

        bool operator<(const SegmentsPoint& other) const {
            double ino = value - other.value;
            if (std::abs(ino) > EPS)
                return ino < 0;
            if (segStatus != other.segStatus)
                return static_cast<int8_t>(segStatus) < static_cast<int8_t>(other.segStatus);
            return this->idSeg < other.idSeg;
        }
    };

    // отсчет по X и соотвествующий отсортированный по Y список проходящих через него рёбер слоя
    struct DiscreteX {
        double value = 0;
        std::vector<SegmentsPoint> masY;
    };

    // структура хранящая данные сегментов слоя
    struct PropertiesSegmentsLayer {
        std::vector<SegmentExt> vecSeg; // список сегментов слоя
        std::vector<DiscreteX>
            masSegX; // список всех отсчетов по X слоя и соотвествующий отсортированный по Y список
                     // проходящих через него рёбер слоя
        std::vector<std::vector<double>>
            vecSegmentsDiscreteY; // список хранящий для каждого ребра слоя массив его значений по Y
                                  // соотвествующий дискретизации отсчетов по X
        std::vector<size_t> linkIndexSegmentMasSeg; // индексу ребра в векторе vecSeg соотвествует
                                                    // индекс его стартового отсчета по X в masSegX
        std::vector<size_t>
            usedSegmntId; // для текущего сегмента записывается его id в сегменты из первого слоя
                          // (для исключения дублирования найденных кандидатов)
        double eps0 = 1e-5;
    };

    // определить ближайший интервал (currentIndexMin, currentIndexMax) индексов
    // включающий значение value в отсортированном массиве vecIntX
    template <typename T>
    static bool FindInterval(double value,
                             std::vector<T>& vecIntX,
                             bool& upOut,
                             size_t& currentIndexMin,
                             size_t& currentIndexMax);

    /**
     * \brief Группировка сегментов слоя
     * \param[in] vecSegLayer набор сегментов слоя
     * \param[out] segLayer структура содержащая информацию о группировке
     */
    static int GroupSegments(const std::vector<SegmentExt>& vecSegLayer,
                             PropertiesSegmentsLayer& segLayer);

    //
    /**
     * \brief получить в контейнер intersectIdSegment части сегментов набора innerIdSegmentL
     * сегментов слоя с текущей областью заданной areaProp \param [in] curSeg текущий сегмент \param
     * [in] innerIdSegmentL набором индексов анализируемых сегментов \param [in] areaProp
     * характеристики области поиска \param [in] extProperties параметры \param [in]
     * directionOffSetEdgeLayer внешнее (true) или внутреннее (fase) направление смещения для ребер
     * слоя \param [in] vecSeg набор сегментов слоя \param [in] eps0 допустимая погрешность \param
     * [in] oppositeMetric флаг использования метрики oppositeMetric \param [out] vecPartSegment
     * список частей сегментов лежащих внутри заданной области
     */
    static void GetCutSegmentsAreaL(const SegmentExt& curSeg,
                                    std::vector<size_t>& innerIdSegmentL,
                                    PropertiesArea& areaProp,
                                    const drc_solver::MeasurementParams& extProperties,
                                    bool directionOffSetEdgeLayer1,
                                    bool directionOffSetEdgeLayer2,
                                    int indexOutContourLayer,
                                    std::vector<SegmentExt>& vecSeg,
                                    double eps0,
                                    bool oppositeMetric,
                                    std::vector<SegmentPart*>& vecPartSegment);

    /**
     * \brief получить в контейнер intersectIdSegment набор сегментов в заданнх границах интервалов
     *  (currentIndexMin, currentIndexMax) отсчета по X в segLayer.masSegX текущего сегмента curSeg
     * \param [in] curSeg текущий сегмент
     * \param [in] areaProp характеристики области поиска
     * \param [in] extProperties параметры
     * \param [in] directionOffSetEdgeLayer внешнее (true) или внутреннее (fase) направление
     * смещения для ребер слоя \param [in] typeMeasurement - тип измерения \param [in] segLayer
     * свойства Слоя \param [in] eps0 допустимая погрешность \param [in] indexOutContourLayer индекс
     * контура слоя охватывающего текущий сегмент (-1 если такого контура нет) \param [in]
     * cutSegment рассматривать все ребра заданного набора (true) или только внутренние рёбра \param
     * [out] innerIdSegment добавляются полученные части сегмента лежащие полностью внутри \param
     * [out] intersectIdSegment добавляются полученные части сегмента
     */
    static void GetSegmentsArea(const SegmentExt& curSeg,
                                PropertiesArea& areaProp,
                                const drc_solver::MeasurementParams& extProperties,
                                bool directionOffSetEdge,
                                bool directionOffSetOpponentEdge,
                                const TypesMeasurement typeMeasurement,
                                PropertiesSegmentsLayer& segLayer,
                                double eps0,
                                int indexOutContourLayer,
                                bool cutSegment,
                                std::vector<size_t>& innerIdSegment,
                                std::vector<size_t>& intersectIdSegment);

    /**
     * \brief Выполнить поиск пар ребер или сегментов ребер многоугольников входных
     * слоев, расстояние между которыми удовлетворяет заданному ограничению
     * \details Вычисление расстояния производится между сторонами ребер входных слоев
     * \param [in] typeMeasurement тип измерения { Ext, Int, Enc }
     * \param [in] extProperties Параметры
     * \param [in] segLayer1 свойства Слоя 1
     * \param [in] vecSeg набор рёбер Слоя 2
     * \param [in] vecEdgeToIndexOutContour в соотвествующий индекс рёбер слоя 2 записывается индекс
     * включающего его контура слоя 1 (-1 если такого контура нет) \param [in] sizeVecSegmentLayer1
     * количество рёбер слоя 1 \param [in] sizeVecSegmentLayer2 количество рёбер слоя 2 \param [in]
     * directionOffSetEdgeLayer1 внешнее (true) или внутреннее (fase) направление смещения для ребер
     * слоя 1 \param [in] directionOffSetEdgeLayer2 внешнее (true) или внутреннее (fase) направление
     * смещения для ребер слоя 2 \param [out] candidatSegmentLayer1ToExtCutEdgeLayer2 // массив
     * хранящий для каждого ребра слоя 1 список своих частей (сегментов), полученных под влиянием
     * рёбер слоя 2 \param [out] candidatSegmentLayer1ToExtEdgeLayer2 // массив хранящий для каждого
     * ребра слоя 2 список сегментов рёбер кандидатов слоя 1 претендующих на попадание в их область
     * расширения \param [out] candidatSegmentLayer1ToExtEdgeLayer1 // массив хранящий для каждого
     * ребра слоя 1 список сегментов рёбер кандидатов слоя 2 претендующих на попадание в их область
     * расширения
     */
    static void MeasurementBuild(const TypesMeasurement typeMeasurement,
                                 const drc_solver::MeasurementParams& extProperties,
                                 PropertiesSegmentsLayer& segLayer1,
                                 std::vector<SegmentExt>& vecSeg,
                                 std::vector<int>& vecEdgeToIndexOutContour,
                                 size_t sizeVecSegmentLayer1,
                                 size_t sizeVecSegmentLayer2,
                                 bool directionOffSetEdgeLayer1,
                                 bool directionOffSetEdgeLayer2,
                                 std::vector<SegmentPart*>* candidatSegmentLayer1ToExtCutEdgeLayer2,
                                 std::vector<size_t>* candidatSegmentLayer1ToIdEdgeLayer2,
                                 std::vector<SegmentPart*>* candidatSegmentLayer1ToExtEdgeLayer2,
                                 std::vector<SegmentPart*>* candidatSegmentLayer2ToExtEdgeLayer1);

    /**
     * \brief Выполнить поиск пар ребер или сегментов ребер многоугольников входных
     * слоев, расстояние между которыми удовлетворяет заданному ограничению
     * \details Вычисление расстояния производится между внешними сторонами ребер
     * входных слоев
     * \param [in] typeMeasurement тип измерения { Ext, Int, Enc }
     * \param [in] extProperties Параметры
     * \param [in] first Слой 1
     * \param [in] second Слой 2
     * \param [in] directionOffSetEdgeLayer1 внешнее (true) или внутреннее (fase) направление
     * смещения для ребер слоя 1 \param [in] directionOffSetEdgeLayer2 внешнее (true) или внутреннее
     * (fase) направление смещения для ребер слоя 2 \param [in] nameBase имя файла для сохранения
     * результата в текстовом виде \return Слой ребер
     */
    static ContourLayer MeasurementBuild(const TypesMeasurement typeMeasurement,
                                         const drc_solver::MeasurementParams& extProperties,
                                         const ContourLayer& first,
                                         const ContourLayer& second,
                                         bool directionOffSetEdgeLayer1 = true,
                                         bool directionOffSetEdgeLayer2 = true,
                                         const std::string& nameBase = "result_measurement");
};

template <typename T>
bool EdgeMeasurement::FindInterval(double value,
                                   std::vector<T>& vecIntX,
                                   bool& upOut,
                                   size_t& currentIndexMin,
                                   size_t& currentIndexMax) {
    currentIndexMin = 0;
    size_t sizevecIntX = vecIntX.size();
    if (sizevecIntX == 0) {
        currentIndexMax = 0;
        upOut = true;
        return false;
    }
    currentIndexMax = sizevecIntX - 1;
    if (vecIntX[currentIndexMin].value > value) // текущее значение меньше минимального интервала
    {
        currentIndexMax = currentIndexMin;
        upOut = true;
        return false;
    }
    if (vecIntX[currentIndexMax].value < value) // текущее значение больше максимального интервала
    {
        currentIndexMin = currentIndexMax;
        upOut = false;
        return false;
    }
    do {
        size_t currentIndex = (currentIndexMin + currentIndexMax) / 2;
        double valueInterval = vecIntX[currentIndex].value;
        if (valueInterval > value) {
            currentIndexMax = currentIndex;
        } else {
            currentIndexMin = currentIndex;
            if (valueInterval == value) {
                currentIndexMax = currentIndex;
                return true;
            }
        }
    } while (currentIndexMax - currentIndexMin > 1);
    return true;
}
} // namespace measurement
#endif // __EDGEMEASUREMENT_HPP
