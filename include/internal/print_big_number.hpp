#ifndef GKERNEL_INTERNAL_PRINT_BIG_NUMBER_HPP
#define GKERNEL_INTERNAL_PRINT_BIG_NUMBER_HPP

#include <string>

namespace gkernel {
namespace internal {

static std::string big(long int number) {
    if (number == 0) {
        return "0";
    }

    bool sign = true;

    if (number < 0) {
        number = -number;
        sign = false;
    }

    std::string res;

    while (number) {
        int val = number % 1000;
        number -= val;
        number /= 1000;
        std::string portion = std::to_string(val);

        while (number && portion.size() < 3) {
            portion = "0" + portion;
        }

        if (val) {
            res = portion + (res.empty() ? "" : "," + res);
        }
    }

    if (!sign) {
        res = "-" + res;
    }

    return res;
}

} // namespace internal
} // namespace gkernel
#endif // GKERNEL_INTERNAL_PRINT_BIG_NUMBER_HPP
