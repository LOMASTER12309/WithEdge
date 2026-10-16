#include <iostream>
#include <sstream>
#include <vector>

#include "base.hpp"
#include "bool_operations.hpp"
#include "params.hpp"
#include "with_edge.cpp"

using namespace gkernel;
using namespace drc_solver;

int main() {
    std::vector<Point> rect1Pts{{0.0, 0.0}, {2.0, 0.0}, {2.0, 2.0}, {0.0, 2.0}};
    std::vector<Point> rect2Pts{{2.0, 2.0}, {2.0, 4.0}, {4.0, 4.0}, {4.0, 2.0}};
    std::vector<Point> rect3Pts{{3.0, 4.0}, {3.0, 6.0}, {5.0, 6.0}, {5.0, 4.0}};

    Contour c1(rect1Pts);
    Contour c2(rect2Pts);
    Contour c3(rect3Pts);

    ContourSet layer1({c1});
    ContourSet layer2({c2});

    ConstrParams params = ConstrParams(1, 10, true, true);
    ContourSet result = withEdge(layer1, layer2);

    std::cout << "Check withEdge: " << (result.size() != 0) << "\n\n";

    std::cout << "Result edge count: " << result.size() << '\n';

    // Print all contours
    for (size_t i = 0; i < result.size(); ++i) {
        const Contour& contour = result[i];
        std::cout << "Contour " << i << " (" << contour.size() << " edges):\n";
        for (const Edge& edge : contour) {
            const Point& bp = edge.getBeginPoint();
            const Point& ep = edge.getEndPoint();
            std::cout << "  Edge: (" << bp.x << ", " << bp.y << ") -> (" << ep.x << ", " << ep.y
                      << ")\n";
        }
    }
    return 0;
}
