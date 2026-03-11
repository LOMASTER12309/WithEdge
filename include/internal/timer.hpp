#ifndef GKERNEL_INTERNAL_TIMER_HPP
#define GKERNEL_INTERNAL_TIMER_HPP

#include <chrono>
#include <ctime>
#include <iostream>
#include <ratio>
#include <string>

namespace gkernel {
namespace internal {

struct timer {
    timer() = delete;
    timer(const char* msg, bool q = true);

    ~timer();

    void reset();
    double elapsed();

    std::chrono::high_resolution_clock::time_point tStart;
    std::string message;
    bool quiet;
};

} // namespace internal
} // namespace gkernel
#endif // GKERNEL_INTERNAL_TIMER_HPP
