#ifndef DRC_FUNCTIONS_HPP
#define DRC_FUNCTIONS_HPP

#include "base.hpp"
#include "params.hpp"

using namespace gkernel;

namespace drc_solver {

/**
 * \brief Выбрать сегменты, меньшая величина угла наклона которых к оси Х системы координат
 * удовлетворяет заданному ограничению
 * \param [in] layer Слой
 * \param [in] params Параметры
 * \return Слой рёбер
 */
EdgeLayer angle(const ContourLayer& layer, const ConstrParams& params);

/**
 * \brief Выбрать контура, соответствующие ограничению площади
 * \param [in] layer Слой
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer area(const ContourLayer& layer, const ConstrParams& params);

/**
 * \brief Выбрать рёбра или части рёбер, совпадающие с ребром из второго слоя
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой ребер
 */
EdgeLayer coincidentEdge(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief Выбрать рёбра или части ребер, совпадающие с ребром или частью ребра из второго слоя и
 * имеющие ту же внутреннюю сторону
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой ребер
 */
EdgeLayer coincidentInsideEdge(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief Выбрать рёбра или части рёбер, совпадающие с ребром или частью ребра из второго слоя и
 * имеющие противоположную внутреннюю сторону
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой ребер
 */
EdgeLayer coincidentOutsideEdge(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief Выбрать контура, которые частично, но не полностью разделяют свою площадь с контурами из
 * второго слоя
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой контуров
 */
ContourLayer cut(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief Создать производный слой контуров путем измерения плотности входного слоя в пределах
 * заданного окна в заданной области
 * \param [in] layer Слой
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer density(const ContourLayer& layer, const DensityParams& params);

/**
 * \brief Выбрать контура, имеющие внутренние циклы (отверстия), которые полностью окружены внешним
 * контуром
 * \param [in] layer Слой
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer donut(const ContourLayer& layer, const ConstrParams* params = nullptr);

/**
 * \brief Выбрать все контура, которые полностью заключают в себе полигон со второго слоя
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer enclose(const ContourLayer& layer1,
                     const ContourLayer& layer2,
                     const ConstrParams* params = nullptr);

/**
 * \brief Выбрать пары рёбер или частей рёбер слоёв, для которых расстояние между внешними сторонами
 * рёбер одного слоя и внутренними сторонами рёбер другого слоя удовлетворяет ограничению
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \param [in] params Параметры
 * \return Слой пар ребер
 */
EdgeClusterLayer enclosureEdges(const ContourLayer& layer1,
                                const ContourLayer& layer2,
                                const MeasurementParams& params);

/**
 * \brief Создать производный слой контуров из пар рёбер или частей рёбер слоёв, для которых
 * расстояние между внешними сторонами рёбер одного слоя и внутренними сторонами рёбер другого слоя
 * удовлетворяет ограничению
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer enclosureRegions(const ContourLayer& layer1,
                              const ContourLayer& layer2,
                              const MeasurementParams& params);

/**
 * \brief Создать производный слой, в котором рёбра контуров расширены в прямоугольники в указанных
 * направлениях и на заданные величины.
 * \param [in] layer Слой
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer expandEdge(const ContourLayer& layer, const ExpandParams& params);

/**
 * \brief Создать производный слой с контуром, представляющим из себя минимальный ограничивающий
 * прямоугольник
 * \param [in] layer Слой
 * \return Слой контуров
 */
ContourLayer extent(const ContourLayer* layer = nullptr);

/**
 * \brief Создать производный слой, состоящий из объединенных минимальных ограничивающих
 * прямоугольников (bounding box) каждого полигона на указанном входном слое
 * \param [in] layer Слой
 * \return Слой контуров
 */
ContourLayer extents(const ContourLayer& layer);

/**
 * \brief Выбрать пары рёбер или частей рёбер слоёв, для которых расстояние между внешними сторонами
 * рёбер одного слоя и внешними сторонами рёбер другого слоя удовлетворяет ограничению
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \param [in] params Параметры
 * \return Слой пар ребер
 */
EdgeClusterLayer externalEdges(const ContourLayer& layer1,
                               const ContourLayer& layer2,
                               const MeasurementParams& params);

/**
 * \brief Создать производный слой контуров из пар рёбер или частей рёбер слоёв, для которых
 * расстояние между внешними сторонами рёбер одного слоя и внешними сторонами рёбер другого слоя
 * удовлетворяет ограничению
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer externalRegions(const ContourLayer& layer1,
                             const ContourLayer& layer2,
                             const MeasurementParams& params);

/**
 * \brief Создать производный слой из контуров, которые помещаются в отверстия контуров
 * входного слоя
 * \param [in] layer Слой
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer holes(const ContourLayer& layer, const ConstrParams* params = nullptr);

/**
 * \brief Выбрать все контура, которые полностью делят всю свою площадь с контурами из второго слоя
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой конутров
 */
ContourLayer inside(const ContourLayer& layer1, const ContourLayer& layer2);

EdgeLayer insideEdge(const EdgeLayer& segLayer);

/**
 * \brief Выбрать рёбра или части рёбер, которые полностью находятся внутри контура из второго слоя
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой ребер
 */
EdgeLayer insideEdge(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief Выбрать все контура, которые имеют общую площадь, ребро или часть ребра с контурами
 * второго слоя
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой контуров
 */
ContourLayer interact(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief Выбрать пары рёбер или частей рёбер слоёв, для которых расстояние между внутренними
 * сторонами рёбер одного слоя и внутренними сторонами рёбер другого слоя удовлетворяет ограничению
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \param [in] params Параметры
 * \return Слой пар ребер
 */
EdgeClusterLayer internalEdges(const ContourLayer& layer1,
                               const ContourLayer& layer2,
                               const MeasurementParams& params);

/**
 * \brief Создать производный слой контуров из пар рёбер или частей рёбер слоёв, для которых
 * расстояние между внутренними сторонами рёбер одного слоя и внутренними сторонами рёбер другого
 * слоя удовлетворяет ограничению
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer internalRegions(const ContourLayer& layer1,
                             const ContourLayer& layer2,
                             const MeasurementParams& params);

/**
 * \brief И
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой контуров
 */
ContourLayer intersection(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief Выбрать рёбра, длина которых удовлетворяет заданному ограничению
 * \param [in] layer Слой
 * \param [in] params Параметры
 * \return Слой ребер
 */
EdgeLayer length(const ContourLayer& layer, const ConstrParams& params);

/**
 * \brief Выбрать рёбра или части рёбер, которые не совпадают с рёбрами со второго слоя
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой ребер
 */
EdgeLayer notCoincidentEdge(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief Выбрать контура, которые делят всю свою внутреннюю площадь или не делят её вовсе с
 * контурами со второго слоя
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой контуров
 */
ContourLayer notCut(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief Выбрать контура входного слоя, которые не имеют внутренних циклов
 * \param [in] layer Слой
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer notDonut(const ContourLayer& layer, const ConstrParams* params = nullptr);

/**
 * \brief Выбрать контура первого слоя, которые не взаимодействуют с контурами второго слоя
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой контуров
 */
ContourLayer notInteract(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief Выбрать рёбра, длина которых не удовлетворяет заданному ограничению
 * \param [in] layer Слой
 * \param [in] params Параметры
 * \return Слой ребер
 */
EdgeLayer notLength(const ContourLayer& layer, const ConstrParams& params);

/**
 * \brief Выбрать все контура первого слоя, которые не лежат полностью вне контуров второго слоя
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой контуров
 */
ContourLayer notOutside(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief Выбрать контура, которые не являются прямоугольниками, с опционально заданными размерами
 * \param [in] layer Слой
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer notRectangle(const ContourLayer& layer, const RectangleParams* params = nullptr);

/**
 * \brief Выбрать контура первого слоя, которые не находятся снаружи всех контуров на втором слое и
 * не имеют общего совпадающего ребра (или его части) с контуром на втором слое
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer notTouch(const ContourLayer& layer1,
                      const ContourLayer& layer2,
                      const ConstrParams* params = nullptr);

/**
 * \brief Выбрать целые рёбра первого слоя, которые не совпадают с ребром или частью ребра второго
 * слоя
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой ребер
 */
EdgeLayer notTouchEdge(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief Выбрать рёбра, не находящиеся на сетке проектирования
 * \param [in] layer Слой
 * \param [in] step Шаг сетки проектирования
 * \return Слой ребер
 */
EdgeLayer offGrid(const ContourLayer& layer, coordType step);

/**
 * \brief Выбрать все контура первого слоя, которые полностью лежат вне контуров второго слоя
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой контуров
 */
ContourLayer outside(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief Выбрать рёбра или части рёбер первого слоя, которые полностью находятся вне контуров
 * второго слоя
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой рёбер
 */
EdgeLayer outsideEdge(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief Выбрать все рёбра на максимальном непрерывном пути рёбер одного контура, общая длина
 * которого (пути) удовлетворяет заданному ограничению
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой рёбер
 */
EdgeLayer pathLength(const ContourLayer& layer, const ConstrParams& params);

/**
 * \brief Выбрать контура, имеющие заданный периметр
 * \param [in] layer Слой
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer perimeter(const ContourLayer& layer, const ConstrParams& params);

/**
 * \brief Выбрать контура, которые являются прямоугольниками, с опционально заданными ограничениями
 * \param [in] layer Слой
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer rectangle(const ContourLayer& layer, const RectangleParams* params = nullptr);

/**
 * \brief Создать производный слой путем расширения или сжатия контуров на указанную величину
 * \param [in] layer Слой
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer size(const ContourLayer& layer, const SizeParams& params);

/**
 * \brief NOT (вычитание)
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой контуров
 */
ContourLayer subtraction(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief Выбрать контура, которые находятся вне всех контуров на втором слое и имеют общую
 * совпадающую грань или её часть с контуром на втором слое
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \param [in] params Параметры
 * \return Слой контуров
 */
ContourLayer touch(const ContourLayer& layer1,
                   const ContourLayer& layer2,
                   const ConstrParams* params = nullptr);

/**
 * \brief Выбрать целые рёбра первого слоя, которые совпадают с ребром или частью ребра из второго
 * слоя
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой ребер
 */
EdgeLayer touchEdge(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief ИЛИ в рамках одного слоя
 * \param [in] layer1 Слой
 * \return Слой контуров
 */
ContourLayer _union(const ContourLayer& layer);

/**
 * \brief ИЛИ
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой контуров
 */
ContourLayer _union(const ContourLayer& layer1, const ContourLayer& layer2);

/**
 * \brief XOR
 * \param [in] layer1 Слой 1
 * \param [in] layer2 Слой 2
 * \return Слой контуров
 */
ContourLayer XOR(const ContourLayer& layer1, const ContourLayer& layer2);

} // namespace drc_solver

#endif // DRC_FUNCTIONS_HPP