#ifndef GKERNEL_INTERNAL_CROSS_RES_HPP
#define GKERNEL_INTERNAL_CROSS_RES_HPP

#include <set>
#include <vector>

#include "base.hpp"

namespace gkernel {
namespace internal {

std::ostream& operator<<(std::ostream& o, const Point& p);

struct EdgePair {
    bool addEdge(const Edge*); // adds segment providing segment1 > segment0, if both segment0
                               // and segment1 are added returns false, otherwise returns true

    const Edge* edge0() const {
        return _edge0;
    }

    const Edge* edge1() const {
        return _edge1;
    }

protected:
    const Edge* _edge0 = nullptr;
    const Edge* _edge1 = nullptr;
};

struct CrossPoint : public Point, public EdgePair {
    CrossPoint() = default;
    CrossPoint(double, double);

    bool operator==(const CrossPoint& cp) {
        return epsEqual(x, cp.x) && epsEqual(y, cp.y) && _edge0 == cp._edge0 && _edge1 == cp._edge1;
    }

    bool operator!=(const CrossPoint& cp) {
        return !(*this == cp);
    }
};

struct CrossOverlap : public EdgePair {
    Point p0, p1;
    CrossOverlap() = default;
    CrossOverlap(double, double, double, double);

    bool operator==(const CrossOverlap& ov) {
        return epsEqual(p0.x, ov.p0.x) && epsEqual(p0.y, ov.p0.y) && epsEqual(p1.x, ov.p1.x) &&
               epsEqual(p1.y, ov.p1.y) && _edge0 == ov._edge0 && _edge1 == ov._edge1;
    }

    bool operator!=(const CrossOverlap& ov) {
        return !(*this == ov);
    }
};

bool operator<(const CrossPoint& cp0, const CrossPoint& cp1);

using CrossRes = std::set<CrossPoint>;

std::vector<CrossPoint> toVector(const CrossRes&);

std::ostream& operator<<(std::ostream& os, const CrossPoint& cp);

struct CrossResVec : public std::vector<CrossPoint> {
    std::vector<CrossOverlap> overlaps;
    void removeDuplicates();
};

} // namespace internal
} // namespace gkernel

#endif // GKERNEL_INTERNAL_CROSS_RES_HPP
