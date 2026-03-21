#include <iostream>
#include <vector>
#include <sstream>

#include "base.hpp"
#include "params.hpp"
#include "bool_operations.hpp"
#include "with_edge.cpp"

using namespace gkernel;
using namespace drc_solver;

int main() {


    std::vector<Point> rect1Pts{{0.0, 0.0}, {4.0, 0.0}, {4.0, 10.0}, {0.0, 10.0}};
    std::vector<Point> rect2Pts{{4.0, 0.0}, {8.0, 0.0}, {8.0, 10.0}, {4.0, 10.0}};
    std::vector<Point> rect3Pts{{3.0, 4.0}, {3.0, 6.0}, {5.0, 6.0}, {5.0, 4.0}};

    Contour c1(rect1Pts);
    Contour c2(rect2Pts);
    Contour c3(rect3Pts);

    ContourSet layer1({c1});
    ContourSet layer2({c2});

    ConstrParams params = ConstrParams(1, 10, true, true);
    ContourSet test = withEdge(layer1, layer2, params);
    
    std::cout << "Проверка withEdge успешна: " << (test.size() == 0) << "\n\n";


    EdgeLayer result = boolAND(layer1, layer2);

    std::cout << "Result edge count: " << result.size() << '\n';
    for (std::size_t i = 0; i < result.size(); ++i) {
        const auto& e = result[i];
        std::cout << "edge[" << i << "]: (" << e.getBeginPoint().x << ", " << e.getBeginPoint().y
                  << ") -> (" << e.getEndPoint().x << ", " << e.getEndPoint().y << ")\n";
    }

    return 0;
}
