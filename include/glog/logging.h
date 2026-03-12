#pragma once

#include <ostream>

namespace google {
inline void InitGoogleLogging(const char*) {}
} // namespace google

namespace gkernel_glog_stub {
class NullStream {
public:
    template <typename T>
    NullStream& operator<<(const T&) {
        return *this;
    }
};
} // namespace gkernel_glog_stub

#define INFO 0
#define LOG(level) gkernel_glog_stub::NullStream()
