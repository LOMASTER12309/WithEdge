#ifndef GKERNEL_CONVERTERS_HPP
#define GKERNEL_CONVERTERS_HPP

#include "base.hpp"

namespace gkernel {
namespace converters {

EdgeLayer toEdgeLayer(const EdgeSet& edges);

ContourLayer toContourLayer(const EdgeLayer& edges);

} // namespace converters
} // namespace gkernel

#endif // GKERNEL_CONVERTERS_HPP
