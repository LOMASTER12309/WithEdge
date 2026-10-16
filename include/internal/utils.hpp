#ifndef UTILS_HPP
#define UTILS_HPP

#include <functional>

#include "base.hpp"
#include "intersection.hpp"
#include "labels.hpp"

namespace std {

template <> struct hash<pair<gkernel::labelDataType, gkernel::labelDataType>> {
    size_t operator()(const pair<gkernel::labelDataType, gkernel::labelDataType>& input) const {
        auto first_hash = hash<gkernel::labelDataType>{}(input.first);
        auto second_hash = hash<gkernel::labelDataType>{}(input.second);
        if (first_hash != second_hash) {
            return first_hash ^ second_hash;
        }
        return first_hash;
    }
};

} // namespace std

namespace drc_solver {

namespace internal {

class IntersectionWatcher {
public:
    IntersectionWatcher(const gkernel::EdgeSet& edgeSet,
                        bool trackFirstLayer = false,
                        bool trackSecondLayer = false);

    void operator()(const gkernel::IntersectionSegment& intersection) {
        if (!intersection.isPoint()) {
            return;
        }

        auto& intersectionPoint = intersection.getFirstPoint();
        auto& firstSegment = edgeSet[intersection.first()];
        auto& secondSegment = edgeSet[intersection.second()];

        if (intersectionPoint == firstSegment.getBeginPoint() ||
            intersectionPoint == firstSegment.getEndPoint() ||
            intersectionPoint == secondSegment.getBeginPoint() ||
            intersectionPoint == secondSegment.getEndPoint()) {
            return;
        }

        intersectionsTracker(intersection.first(), intersection.second());
    }

    const std::unordered_set<gkernel::labelDataType>& getFirstLayerIntersections() const {
        return firstLayerIntersections;
    }

    const std::unordered_set<gkernel::labelDataType>& getSecondLayerIntersections() const {
        return secondLayerIntersections;
    }

private:
    const std::vector<gkernel::labelDataType>* layerID;
    const std::vector<gkernel::labelDataType>* contourID;
    const gkernel::EdgeSet& edgeSet;

    std::unordered_set<gkernel::labelDataType> firstLayerIntersections;
    std::unordered_set<gkernel::labelDataType> secondLayerIntersections;
    std::function<void(std::size_t, std::size_t)> intersectionsTracker;
};

} // namespace internal

} // namespace drc_solver

#endif // UTILS_HPP
