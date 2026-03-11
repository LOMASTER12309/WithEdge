#ifndef GKERNEL_BOOL_OPERATIONS_HPP
#define GKERNEL_BOOL_OPERATIONS_HPP

#include "base.hpp"

namespace gkernel {

EdgeLayer boolAND(const ContourSet& lhs, const ContourSet& rhs);

EdgeLayer boolOR(const EdgeLayer& layer);

EdgeLayer boolOR(const ContourSet& lhs, const ContourSet& rhs);

EdgeLayer boolNOT(const ContourSet& lhs, const ContourSet& rhs);

EdgeLayer boolXOR(const ContourSet& lhs, const ContourSet& rhs);

} // namespace gkernel

#endif // GKERNEL_BOOL_OPERATIONS_HPP
