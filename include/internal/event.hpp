#ifndef INTERNAL_EVENT_HPP
#define INTERNAL_EVENT_HPP

#include <cmath>

#include "base.hpp"

namespace gkernel {
namespace internal {

template <typename Status> struct Event {
    Event(double x, std::size_t segmentIdx, Status status)
        : x(x), segmentIdx(segmentIdx), status(status) {}

    double x;
    std::size_t segmentIdx;
    Status status;
};

} // namespace internal
} // namespace gkernel

#endif // INTERNAL_EVENT_HPP
