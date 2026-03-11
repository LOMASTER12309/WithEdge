#ifndef FILTERS_HPP
#define FILTERS_HPP

#include <functional>
#include <unordered_set>

#include "base.hpp"
#include "types.hpp"

using namespace gkernel;

namespace drc_solver {

using Predicate = std::function<bool(std::size_t value)>;
using LayersFunctionTwoLayers = std::function<std::unordered_set<gkernel::labelDataType>(
    const ContourLayer& first, const ContourLayer& second, Predicate pred)>;
using LayersFunction = std::function<std::unordered_set<gkernel::labelDataType>(
    const ContourLayer& layer, Predicate pred)>;

ContourLayer filterIDs(const ContourLayer& layer1,
                       const ContourLayer& layer2,
                       LayersFunctionTwoLayers func,
                       Predicate pred = {});

ContourLayer filterNotIDs(const ContourLayer& layer1,
                          const ContourLayer& layer2,
                          LayersFunctionTwoLayers func,
                          Predicate pred = {});

ContourLayer filterIDs(const ContourLayer& layer, LayersFunction func, Predicate pred = {});

ContourLayer filterNotIDs(const ContourLayer& layer, LayersFunction func, Predicate pred = {});
} // namespace drc_solver

#endif // FILTERS_HPP