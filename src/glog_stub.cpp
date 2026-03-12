#include <ostream>
#include <sstream>

namespace google {

class LogMessage {
public:
    LogMessage(const char*, int);
    LogMessage(const char*, int, int);
    ~LogMessage();
    std::ostream& stream();

private:
    std::ostringstream buffer;
};

LogMessage::LogMessage(const char*, int) {}
LogMessage::LogMessage(const char*, int, int) {}
LogMessage::~LogMessage() = default;
std::ostream& LogMessage::stream() {
    return buffer;
}

} // namespace google
