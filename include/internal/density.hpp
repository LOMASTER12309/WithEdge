#ifndef INTERNAL_DENSITY_HPP
#define INTERNAL_DENSITY_HPP

#include <vector>

#include "base.hpp"

namespace drc_solver {
namespace internal {

std::vector<double> calculateDensity(const gkernel::ContourLayer& layer,
                                     std::size_t numCellsX,
                                     std::size_t numCellsY);

} // namespace internal
} // namespace drc_solver

#endif // INTERNAL_DENSITY_HPP
