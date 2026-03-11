#ifndef GKERNEL_HELPERS_HPP
#define GKERNEL_HELPERS_HPP

#include "base.hpp"

namespace gkernel {

static inline double getSweepingLineY(const Edge& segment, double x, double eps = 0) {
    double k = (segment.getEndPoint().y - segment.getBeginPoint().y) /
               (segment.getEndPoint().x - segment.getBeginPoint().x);
    double m = segment.getBeginPoint().y - k * segment.getBeginPoint().x;

    return k * (x + eps) + m;
}

enum OverlapPolicy { SKIP, PREFER_FIRST, PREFER_SECOND };

void copyLabels(const EdgeSet& from,
                const EdgeSet& to,
                std::initializer_list<labelType> labels,
                OverlapPolicy policy);

} // namespace gkernel

#endif // GKERNEL_HELPERS_HPP
