#include "base.hpp"
#include "params.hpp"

using namespace gkernel;
using namespace drc_solver;

//в params.hpp это чисто виртуальная функция без тела
//поэтому без её определение не выйдет
bool ConstrParams::init(std::string str) {
    return true;
}

ContourLayer withEdge(const ContourLayer& layer1, const ContourLayer& layer2, const ConstrParams& params) {
    return ContourSet{};
}