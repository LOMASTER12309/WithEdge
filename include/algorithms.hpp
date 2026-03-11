#ifndef GKERNEL_ALGORITHMS_HPP
#define GKERNEL_ALGORITHMS_HPP

#include <cmath>
#include <map>
#include <vector>

#include "base.hpp"

namespace gkernel {

EdgeLayer findSegmentsNeighbours(const EdgeLayer& inputLayer);

void markAreas(const EdgeSet& layer);

std::vector<Contour> extractContours(const std::map<Point, std::vector<Point>>& adjacency);

void markChilds(const ContourLayer& layer);

} // namespace gkernel

#endif // GKERNEL_ALGORITHMS_HPP
