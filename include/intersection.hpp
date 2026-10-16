#ifndef GKERNEL_INTERSECTION_HPP
#define GKERNEL_INTERSECTION_HPP

#include <functional>
#include <utility>
#include <vector>

#include "base.hpp"

namespace gkernel {

using Watcher = std::function<void(const Edge& first, const Edge& second)>;

struct IntersectionSegment {
    IntersectionSegment(const Point& firstPoint,
                        std::size_t _firstEdgeIdx,
                        std::size_t _secondEdgeIdx)
        : _isPoint(true), firstPoint(firstPoint), secondPoint(firstPoint),
          firstEdgeIdx(_firstEdgeIdx), secondEdgeIdx(_secondEdgeIdx) {}

    IntersectionSegment(const Point& firstPoint,
                        const Point& secondPoint,
                        std::size_t _firstEdgeIdx,
                        std::size_t _secondEdgeIdx)
        : _isPoint(false), firstPoint(firstPoint), secondPoint(secondPoint),
          firstEdgeIdx(_firstEdgeIdx), secondEdgeIdx(_secondEdgeIdx) {}

    const Point& getFirstPoint() const {
        return firstPoint;
    }

    const Point& getSecondPoint() const {
        return secondPoint;
    }

    std::size_t first() const {
        return firstEdgeIdx;
    }

    std::size_t second() const {
        return secondEdgeIdx;
    }

    bool isPoint() const {
        return _isPoint;
    }

    void reverse() {
        std::swap(firstPoint, secondPoint);
    }

    int firstOverlapEdgeId = -1;
    int secondOverlapEdgeId = -1;

private:
    bool _isPoint = false;
    Point firstPoint;
    Point secondPoint;
    std::size_t firstEdgeIdx;
    std::size_t secondEdgeIdx;
};

class Intersection {
public:
    enum Algorithms { SWEEP_LINE, SWEEP_LINE_NEW, TREE, N2 };

    static Algorithms algorithm;

    static std::vector<IntersectionSegment> intersectEdgeSet(const EdgeSet& edges,
                                                             Watcher intersectionWatcher = {},
                                                             Watcher overlapWatcher = {});
};

} // namespace gkernel
#endif // GKERNEL_INTERSECTION_HPP
