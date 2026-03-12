#include <iostream>
#include <vector>
#include <sstream>

#include "base.hpp"
#include "bool_operations.hpp"

int main() {
    using namespace gkernel;

    std::vector<Point> rect1Pts{{0.0, 0.0}, {4.0, 0.0}, {4.0, 3.0}, {0.0, 3.0}};
    std::vector<Point> rect2Pts{{2.0, 1.0}, {6.0, 1.0}, {6.0, 4.0}, {2.0, 4.0}};

    Contour c1(rect1Pts);
    Contour c2(rect2Pts);

    ContourSet layer1({c1});
    ContourSet layer2({c2});

    EdgeLayer result = boolAND(layer1, layer2);

    std::cout << "Result edge count: " << result.size() << '\n';
    for (std::size_t i = 0; i < result.size(); ++i) {
        const auto& e = result[i];
        std::cout << "edge[" << i << "]: (" << e.getBeginPoint().x << ", " << e.getBeginPoint().y
                  << ") -> (" << e.getEndPoint().x << ", " << e.getEndPoint().y << ")\n";
    }

    return 0;
}
