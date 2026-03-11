#ifndef GKERNEL_INTERNAL_BINTERSECT_HPP
#define GKERNEL_INTERNAL_BINTERSECT_HPP

#include <array>
#include <ostream>

#include "base.hpp"

namespace gkernel {
namespace internal {

std::array<double, 4> intersect(const Edge& seg0, const Edge& seg1);

std::ostream& operator<<(std::ostream& os, const Edge& seg);

} // namespace internal
} // namespace gkernel

#endif // GKERNEL_INTERNAL_BINTERSECT_HPP
