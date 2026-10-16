#ifndef GKERNEL_INTERSECTION_SWEEP_LINE_HPP
#define GKERNEL_INTERSECTION_SWEEP_LINE_HPP

#include "base.hpp"
#include "intersection.hpp"

namespace gkernel {

class IntersectionSWPL {
public:
    IntersectionSWPL() = delete;

    static Point intersectEdges(const Edge& first,
                                const Edge& second,
                                Watcher intersectionWatcher = {});

    static std::pair<Point, Point> overlapEdges(const Edge& first,
                                                const Edge& second,
                                                Watcher overlapWatcher = {});

    static std::pair<Point, Point> overlapEdgesVertical(const Edge& first,
                                                        const Edge& second,
                                                        Watcher overlapWatcher = {});

    static std::vector<IntersectionSegment> intersectEdgeSet(const EdgeSet& segments,
                                                             Watcher intersectionWatcher = {},
                                                             Watcher overlapWatcher = {});
};

} // namespace gkernel

#endif // GKERNEL_INTERSECTION_SWEEP_LINE_HPP
