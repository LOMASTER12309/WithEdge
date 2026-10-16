#include <algorithm>
#include <cmath>
#include <future>
#include <parallel/algorithm>
#include <thread>
#include <vector>

#include "base.hpp"
#include "line_helper.cpp"
#include "params.hpp"

using namespace gkernel;
using namespace constants;
using namespace drc_solver;

bool IsEqual(double x, double y, double eps) {
    return std::abs(x - y) < eps;
}

// Компаратор для сортировки ребер по идентификатору и левому концу
// Нормали квантованы, поэтому сравниваются точно
bool EdgeCompare(const EdgeData &first, const EdgeData &second) noexcept {
    if (first.line.normalX != second.line.normalX) {
        return first.line.normalX < second.line.normalX;
    }
    if (first.line.normalY != second.line.normalY) {
        return first.line.normalY < second.line.normalY;
    }
    if (!IsEqual(first.line.signedDistance, second.line.signedDistance, eps)) {
        return first.line.signedDistance < second.line.signedDistance;
    }
    return first.line.leftInterval < second.line.leftInterval;
}

// Для второго слоя индекс контура не нужен,
// храним только идентификатор опорной прямой
struct LineData {
    LineData() noexcept {
    }

    LineData(const LineId &lineId) noexcept : line(lineId) {
    }

    LineId line;
};

// Блок - непрерывный диапазон контуров, который обрабатывает один поток.
// offset указывает, с какого места в общем массиве рёбер поток может писать.
// Диапазоны offset не пересекаются, поэтому синхронизация при записи не нужна.
struct BuildBlock {
    std::size_t beginContour;
    std::size_t endContour;
    std::size_t offset;
};

// Такой же порядок сортировки, как у EdgeCompare, но для облегченных записей layer2.
bool LineDataCompare(const LineData &first, const LineData &second) noexcept {
    if (first.line.normalX != second.line.normalX) {
        return first.line.normalX < second.line.normalX;
    }
    if (first.line.normalY != second.line.normalY) {
        return first.line.normalY < second.line.normalY;
    }
    if (!IsEqual(first.line.signedDistance, second.line.signedDistance, eps)) {
        return first.line.signedDistance < second.line.signedDistance;
    }
    return first.line.leftInterval < second.line.leftInterval;
}

// Порог для количества рёбер в слое, при котором имеет смысл использовать многопоточность для
// формирования записей
constexpr std::size_t kMinBuildEdgesPerWorker = 50000;

// Определяет кол-во потоков для формирования записей EdgeData для layer1.
std::size_t CalcBuildTheadCount(std::size_t contourCount, std::size_t edgeCount) noexcept {
    // Для маленьких входов async дороже, чем последовательный проход:
    // создание задач и последующая компактация не успевают окупиться.
    if (contourCount == 0 || edgeCount < kMinBuildEdgesPerWorker) {
        return 1;
    }

    // Используем аппаратную параллельность как верхнюю границу.
    // Если среда не смогла её определить, берём 2 потока.
    std::size_t theadCount = std::thread::hardware_concurrency();
    if (theadCount == 0) {
        theadCount = 2;
    }

    // Не создаём больше потоков, чем контуров, и не дробим работу слишком мелко.
    // Нижняя граница рёбер на поток снижает накладные расходы на async.
    const std::size_t edgeThreadCount = edgeCount / kMinBuildEdgesPerWorker;
    theadCount = std::min(theadCount, contourCount);
    theadCount = std::min(theadCount, std::max<std::size_t>(edgeThreadCount, 1));
    return theadCount;
}

// Функция определяет блоки (начало и конец) для каждого потока
std::vector<BuildBlock> MakeBuildBlocks(const ContourLayer &layer, std::size_t theadCount) {
    std::vector<BuildBlock> blocks;
    blocks.reserve(theadCount);

    // Раздаём каждому потоку примерно равное количество контуров. Рёбра внутри контура не дробятся.
    std::size_t offset = 0;
    for (std::size_t workerIndex = 0; workerIndex < theadCount; ++workerIndex) {
        const std::size_t beginContour = layer.size() * workerIndex / theadCount;
        const std::size_t endContour = layer.size() * (workerIndex + 1) / theadCount;

        // Считаем верхнюю границу числа записей блока.
        // Реальных записей может быть меньше, если MakeLineId отбросит вырожденные рёбра.
        std::size_t blockEdgeCount = 0;
        for (std::size_t contourIndex = beginContour; contourIndex < endContour; ++contourIndex) {
            blockEdgeCount += layer[contourIndex].size();
        }

        blocks.push_back(BuildBlock{beginContour, endContour, offset});
        offset += blockEdgeCount;
    }

    return blocks;
}

std::vector<EdgeData> BuildLayer1Records(const ContourLayer &layer, std::size_t edgeCount) {
    const std::size_t theadCount = CalcBuildTheadCount(layer.size(), edgeCount);

    // 1 рабочий поток, простой последовательный проход
    if (theadCount == 1) {
        std::vector<EdgeData> records;
        records.reserve(edgeCount);

        for (std::size_t contourIndex = 0; contourIndex < layer.size(); ++contourIndex) {
            const Contour &contour = layer[contourIndex];
            LineId lineId;

            for (const Edge &edge : contour) {
                if (!MakeLineId(edge, lineId)) {
                    continue;
                }
                records.push_back(EdgeData{lineId, contourIndex});
            }
        }

        return records;
    }

    std::vector<BuildBlock> blocks = MakeBuildBlocks(layer, theadCount);

    // Выделяем общий массив под максимальное число записей.
    // Каждый поток пишет в свой непересекающийся диапазон, заданный BuildBlock::offset.
    std::vector<EdgeData> records(edgeCount);
    std::vector<std::future<std::size_t>> tasks;
    tasks.reserve(blocks.size());

    for (std::size_t blockIndex = 0; blockIndex < blocks.size(); ++blockIndex) {
        const BuildBlock block = blocks[blockIndex];
        tasks.push_back(std::async(std::launch::async, [&layer, &records, block]() {
            std::size_t writeIndex = block.offset;
            LineId lineId;

            // contourIndex нужен только для layer1: по нему позже отмечается,
            // какой исходный контур должен попасть в результат.
            for (std::size_t contourIndex = block.beginContour; contourIndex < block.endContour;
                 ++contourIndex) {
                const Contour &contour = layer[contourIndex];

                for (const Edge &edge : contour) {
                    if (!MakeLineId(edge, lineId)) {
                        continue;
                    }
                    records[writeIndex++] = EdgeData{lineId, contourIndex};
                }
            }

            return writeIndex - block.offset;
        }));
    }

    std::vector<std::size_t> writtenCounts(blocks.size(), 0);
    std::size_t totalWritten = 0;
    for (std::size_t blockIndex = 0; blockIndex < tasks.size(); ++blockIndex) {
        writtenCounts[blockIndex] = tasks[blockIndex].get();
        totalWritten += writtenCounts[blockIndex];
    }

    // Если вырожденных рёбер не было, records уже плотно заполнен.
    // Иначе между блоками могут остаться "дыры", потому что каждому блоку заранее
    // выделялся диапазон под все его рёбра. Сдвигаем фактически записанные части влево.
    if (totalWritten != edgeCount) {
        std::size_t newOffset = 0;
        for (std::size_t blockIndex = 0; blockIndex < blocks.size(); ++blockIndex) {
            const std::size_t oldOffset = blocks[blockIndex].offset;
            const std::size_t count = writtenCounts[blockIndex];
            if (oldOffset != newOffset && count > 0) {
                std::move(records.begin() + oldOffset, records.begin() + oldOffset + count,
                          records.begin() + newOffset);
            }
            newOffset += count;
        }
        records.resize(totalWritten);
    }

    return records;
}

std::vector<LineData> BuildLayer2Records(const ContourLayer &layer, std::size_t edgeCount) {
    const std::size_t theadCount = CalcBuildTheadCount(layer.size(), edgeCount);

    // Для layer2 последовательный путь также оставлен без лишней параллельной обвязки.
    if (theadCount == 1) {
        std::vector<LineData> records;
        records.reserve(edgeCount);

        for (std::size_t contourIndex = 0; contourIndex < layer.size(); ++contourIndex) {
            const Contour &contour = layer[contourIndex];
            LineId lineId;

            for (const Edge &edge : contour) {
                if (!MakeLineId(edge, lineId)) {
                    continue;
                }
                records.push_back(LineData{lineId});
            }
        }

        return records;
    }

    std::vector<BuildBlock> blocks = MakeBuildBlocks(layer, theadCount);

    // В отличие от layer1, здесь не храним contourIndex:
    // при поиске совпадений нам важны только сами опорные прямые и интервалы layer2.
    std::vector<LineData> records(edgeCount);
    std::vector<std::future<std::size_t>> tasks;
    tasks.reserve(blocks.size());

    for (std::size_t blockIndex = 0; blockIndex < blocks.size(); ++blockIndex) {
        const BuildBlock block = blocks[blockIndex];
        tasks.push_back(std::async(std::launch::async, [&layer, &records, block]() {
            std::size_t writeIndex = block.offset;
            LineId lineId;

            for (std::size_t contourIndex = block.beginContour; contourIndex < block.endContour;
                 ++contourIndex) {
                const Contour &contour = layer[contourIndex];

                for (const Edge &edge : contour) {
                    if (!MakeLineId(edge, lineId)) {
                        continue;
                    }
                    records[writeIndex++] = LineData{lineId};
                }
            }

            return writeIndex - block.offset;
        }));
    }

    std::vector<std::size_t> writtenCounts(blocks.size(), 0);
    std::size_t totalWritten = 0;
    for (std::size_t blockIndex = 0; blockIndex < tasks.size(); ++blockIndex) {
        writtenCounts[blockIndex] = tasks[blockIndex].get();
        totalWritten += writtenCounts[blockIndex];
    }

    // Компактация такая же, как в BuildLayer1Records:
    // нужна только если MakeLineId пропустил часть рёбер.
    if (totalWritten != edgeCount) {
        std::size_t newOffset = 0;
        for (std::size_t blockIndex = 0; blockIndex < blocks.size(); ++blockIndex) {
            const std::size_t oldOffset = blocks[blockIndex].offset;
            const std::size_t count = writtenCounts[blockIndex];
            if (oldOffset != newOffset && count > 0) {
                std::move(records.begin() + oldOffset, records.begin() + oldOffset + count,
                          records.begin() + newOffset);
            }
            newOffset += count;
        }
        records.resize(totalWritten);
    }

    return records;
}

// Компаратор для сравнения идентификаторов прямых
bool LineCompare(const LineId &first, const LineId &second) noexcept {
    if (first.normalX != second.normalX) {
        return first.normalX < second.normalX;
    }
    if (first.normalY != second.normalY) {
        return first.normalY < second.normalY;
    }
    if (!IsEqual(first.signedDistance, second.signedDistance, eps)) {
        return first.signedDistance < second.signedDistance;
    }
    return false;
}

// Провека равентсва индентификаторов (рёбра лежат на одной прямой)
bool IsSameLine(const LineId &first, const LineId &second) noexcept {
    return first.normalX == second.normalX && first.normalY == second.normalY &&
           IsEqual(first.signedDistance, second.signedDistance, eps);
}

// Проверка пересечения с учетом машинной неточности
inline bool HasPositiveCommonPart(double firstLeft, double firstRight, double secondLeft,
                                  double secondRight) noexcept {
    return firstLeft < secondRight - 0.1 * eps && secondLeft < firstRight - 0.1 * eps;
}

// Основной алгоритм WithEdge
ContourLayer withEdge(const ContourLayer &layer1, const ContourLayer &layer2,
                      const ConstrParams *params = nullptr) {
    const std::size_t layer1EdgeCount = CountLayerEdges(layer1);
    const std::size_t layer2EdgeCount = CountLayerEdges(layer2);

    // Если хотя бы один слой не содержит рёбер, общих рёбер быть не может
    // Без params результат пустой. С params нужно проверить, подходит ли 0 общих рёбер
    if (layer1EdgeCount == 0 || layer2EdgeCount == 0) {
        if (params != nullptr && satisfy(0.0, *params)) {
            return layer1;
        }
        return ContourLayer();
    }

    std::vector<EdgeData> layer1Records = BuildLayer1Records(layer1, layer1EdgeCount);
    std::vector<LineData> layer2Records = BuildLayer2Records(layer2, layer2EdgeCount);

    // Параллельная GNU-сортировка в отдельных потоках,
    // каждый слой сортируется GNU parallel sort, а две сортировки запускаются одновременно.
    std::future<void> layer1Sort = std::async(std::launch::async, [&layer1Records]() {
        __gnu_parallel::sort(layer1Records.begin(), layer1Records.end(), EdgeCompare);
    });
    __gnu_parallel::sort(layer2Records.begin(), layer2Records.end(), LineDataCompare);
    layer1Sort.get();

    std::vector<std::size_t> commonEdgeCounts;
    std::vector<unsigned char> commonContourFlags;
    if (params == nullptr) {
        // Без params важно только наличие хотя бы одного общего ребра у контура
        // Флаг дешевле счётчика и позволяет пропускать уже найденные контуры
        commonContourFlags.assign(layer1.size(), 0);
    } else {
        // С params нужно точное число общих рёбер, потому что satisfy проверяет диапазон
        commonEdgeCounts.assign(layer1.size(), 0);
    }

    std::size_t layer1Index = 0;
    std::size_t layer2Index = 0;

    // Сливаем два отсортированных массива по группам одинаковой прямой
    while (layer1Index < layer1Records.size() && layer2Index < layer2Records.size()) {

        if (LineCompare(layer1Records[layer1Index].line, layer2Records[layer2Index].line)) {
            const LineId &lineId = layer1Records[layer1Index].line;

            do {
                ++layer1Index;
            } while (layer1Index < layer1Records.size() &&
                     IsSameLine(lineId, layer1Records[layer1Index].line));
            continue;
        }
        if (!IsSameLine(layer1Records[layer1Index].line, layer2Records[layer2Index].line)) {
            const LineId &lineId = layer2Records[layer2Index].line;

            do {
                ++layer2Index;
            } while (layer2Index < layer2Records.size() &&
                     IsSameLine(lineId, layer2Records[layer2Index].line));
            continue;
        }

        // Имеем совпадение прямых в 1 и 2 слоях, формируем группы рёбер
        const LineId &lineId = layer1Records[layer1Index].line;

        std::size_t layer1GroupEnd = layer1Index + 1;
        while (layer1GroupEnd < layer1Records.size() &&
               IsSameLine(lineId, layer1Records[layer1GroupEnd].line)) {
            ++layer1GroupEnd;
        }

        std::size_t layer2GroupEnd = layer2Index + 1;
        while (layer2GroupEnd < layer2Records.size() &&
               IsSameLine(lineId, layer2Records[layer2GroupEnd].line)) {
            ++layer2GroupEnd;
        }

        std::size_t currentLayer2Index = layer2Index;

        // Обе группы уже отсортированы по leftInterval,
        // поэтому внутри для групп одной прямой делается двухуказательный проход
        for (std::size_t recordIndex = layer1Index; recordIndex < layer1GroupEnd; ++recordIndex) {
            const EdgeData &layer1Record = layer1Records[recordIndex];

            // В режиме без params повторная проверка рёбер уже найденного контура
            // не меняет результат, поэтому её можно безопасно пропустить
            if (params == nullptr && commonContourFlags[layer1Record.contourIndex] != 0) {
                continue;
            }

            const double layer1Left = layer1Record.line.leftInterval;
            const double layer1Right = layer1Record.line.rightInterval;

            while (currentLayer2Index < layer2GroupEnd &&
                   layer2Records[currentLayer2Index].line.rightInterval <= layer1Left + eps) {
                ++currentLayer2Index;
            }

            if (currentLayer2Index == layer2GroupEnd) {
                break;
            }

            const LineId &layer2Line = layer2Records[currentLayer2Index].line;

            if (HasPositiveCommonPart(layer1Left, layer1Right, layer2Line.leftInterval,
                                      layer2Line.rightInterval)) {
                if (params == nullptr) {
                    commonContourFlags[layer1Record.contourIndex] = 1;
                } else {
                    ++commonEdgeCounts[layer1Record.contourIndex];
                }
            }
        }

        layer1Index = layer1GroupEnd;
        layer2Index = layer2GroupEnd;
    }

    // Формирование результата
    std::vector<Contour> resultContours;

    if (params == nullptr) {
        // Сначала считаем точный размер результата.
        // Это даёт точный reserve и позволяет быстро обработать пустой/полный результат
        std::size_t resultCount = 0;
        for (std::size_t contourIndex = 0; contourIndex < layer1.size(); ++contourIndex) {
            if (commonContourFlags[contourIndex] != 0) {
                ++resultCount;
            }
        }

        if (resultCount == 0) {
            return ContourLayer();
        }
        if (resultCount == layer1.size()) {
            return layer1;
        }

        // Здесь копируются только реально выбранные контуры
        resultContours.reserve(resultCount);
        for (std::size_t contourIndex = 0; contourIndex < layer1.size(); ++contourIndex) {
            if (commonContourFlags[contourIndex] != 0) {
                resultContours.push_back(layer1[contourIndex]);
            }
        }
    } else {
        // При наличии params результат определяется не фактом совпадения,
        // а попаданием числа общих рёбер в заданное условие
        std::size_t resultCount = 0;
        for (std::size_t contourIndex = 0; contourIndex < layer1.size(); ++contourIndex) {
            if (satisfy(static_cast<double>(commonEdgeCounts[contourIndex]), *params)) {
                ++resultCount;
            }
        }

        if (resultCount == 0) {
            return ContourLayer();
        }
        if (resultCount == layer1.size()) {
            return layer1;
        }

        resultContours.reserve(resultCount);
        for (std::size_t contourIndex = 0; contourIndex < layer1.size(); ++contourIndex) {
            if (satisfy(static_cast<double>(commonEdgeCounts[contourIndex]), *params)) {
                resultContours.push_back(layer1[contourIndex]);
            }
        }
    }

    return MakeContourLayer(resultContours);
}
