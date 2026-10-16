#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "base.hpp"
#include "params.hpp"
#include "../src/with_edge.cpp"

using namespace gkernel;
using namespace drc_solver;

struct TestSettings {
    int rows;
    int cols;
    int cellWidth;
    int cellHeight;
    double minScale;
    double maxScale;
    ptrdiff_t seed1;
    ptrdiff_t seed2;
};

struct MemorySnapshot {
    std::size_t vmRssKb;
    std::size_t vmHwmKb;
};

typedef ContourLayer (*WithEdgeFunction)(const ContourLayer &, const ContourLayer &,
                                         const ConstrParams *);

static Contour MakeRectContour(double x1, double y1, double x2, double y2) {
    std::vector<Point> pts{{x1, y1}, {x2, y1}, {x2, y2}, {x1, y2}};
    return Contour(pts);
}

static ContourLayer MakeRandomLayer(int rows, int cols, int cellWidth, int cellHeight,
                                    double minScale, double maxScale, ptrdiff_t seed) {
    const int x0 = 0;
    const int y0 = 0;

    std::vector<Contour> contours;
    contours.reserve(rows * cols);

    std::mt19937 gen(seed);

    const int minW = static_cast<int>(cellWidth * minScale);
    const int maxW = static_cast<int>(cellWidth * maxScale);
    const int minH = static_cast<int>(cellHeight * minScale);
    const int maxH = static_cast<int>(cellHeight * maxScale);

    std::uniform_int_distribution<int> wDist(minW, maxW);
    std::uniform_int_distribution<int> hDist(minH, maxH);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            const int cellX = x0 + j * cellWidth;
            const int cellY = y0 + i * cellHeight;

            const int w = wDist(gen);
            const int h = hDist(gen);

            std::uniform_int_distribution<int> dxDist(0, cellWidth - w);
            std::uniform_int_distribution<int> dyDist(0, cellHeight - h);

            const int dx = dxDist(gen);
            const int dy = dyDist(gen);

            const int x1 = cellX + dx;
            const int y1 = cellY + dy;

            contours.push_back(MakeRectContour(x1, y1, x1 + w, y1 + h));
        }
    }

    ContourLayer layer;
    layer.addContours(contours);
    return layer;
}

static bool ReadStatusValue(const std::string &line, const std::string &name,
                            std::size_t &valueKb) {
    if (line.compare(0, name.size(), name) != 0) {
        return false;
    }

    std::istringstream stream(line.substr(name.size()));
    stream >> valueKb;
    return true;
}

static MemorySnapshot ReadMemorySnapshot() {
    std::ifstream status("/proc/self/status");
    std::string line;

    MemorySnapshot snapshot{0, 0};
    while (std::getline(status, line)) {
        ReadStatusValue(line, "VmRSS:", snapshot.vmRssKb);
        ReadStatusValue(line, "VmHWM:", snapshot.vmHwmKb);
    }

    return snapshot;
}

static double KbToMb(std::size_t valueKb) {
    return static_cast<double>(valueKb) / 1024.0;
}

static long long MicrosecondsSince(const std::chrono::steady_clock::time_point &start,
                                   const std::chrono::steady_clock::time_point &finish) {
    return std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();
}

static void PrintMemory(const std::string &label, const MemorySnapshot &snapshot) {
    std::cout << label << ": rss=" << KbToMb(snapshot.vmRssKb)
              << " MB, hwm=" << KbToMb(snapshot.vmHwmKb) << " MB\n";
}

static int RunMeasuredCase(const std::string &name, const TestSettings &settings,
                           WithEdgeFunction function) {
    const auto totalStart = std::chrono::steady_clock::now();
    const MemorySnapshot startMemory = ReadMemorySnapshot();

    const auto generateStart = std::chrono::steady_clock::now();
    ContourLayer layer1 = MakeRandomLayer(settings.rows, settings.cols, settings.cellWidth,
                                          settings.cellHeight, settings.minScale,
                                          settings.maxScale, settings.seed1);
    ContourLayer layer2 = MakeRandomLayer(settings.rows, settings.cols, settings.cellWidth,
                                          settings.cellHeight, settings.minScale,
                                          settings.maxScale, settings.seed2);
    const auto generateFinish = std::chrono::steady_clock::now();
    const MemorySnapshot afterGenerateMemory = ReadMemorySnapshot();

    const auto functionStart = std::chrono::steady_clock::now();
    ContourLayer result = function(layer1, layer2, nullptr);
    const auto functionFinish = std::chrono::steady_clock::now();
    const MemorySnapshot afterFunctionMemory = ReadMemorySnapshot();

    const std::size_t extraPeakKb =
        afterFunctionMemory.vmHwmKb > afterGenerateMemory.vmRssKb
            ? afterFunctionMemory.vmHwmKb - afterGenerateMemory.vmRssKb
            : 0;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n=== " << name << " ===\n";
    std::cout << "input: rows=" << settings.rows << ", cols=" << settings.cols
              << ", contours/layer=" << layer1.size() << ", edges/layer="
              << layer1.size() * 4 << "\n";
    std::cout << "seeds: " << settings.seed1 << ", " << settings.seed2 << "\n";
    PrintMemory("start", startMemory);
    PrintMemory("after layer generation", afterGenerateMemory);
    PrintMemory("after function", afterFunctionMemory);
    std::cout << "extra peak over generated layers: " << KbToMb(extraPeakKb) << " MB\n";
    std::cout << "result contours: " << result.size() << "\n";
    std::cout << "generate time: " << MicrosecondsSince(generateStart, generateFinish) << " us\n";
    std::cout << "function time: " << MicrosecondsSince(functionStart, functionFinish) << " us\n";
    std::cout << "total child time: " << MicrosecondsSince(totalStart, functionFinish) << " us\n";
    std::cout << std::flush;

    return 0;
}

static int RunChild(const std::string &name, const TestSettings &settings,
                    WithEdgeFunction function) {
    const pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "fork failed for " << name << "\n";
        return 1;
    }

    if (pid == 0) {
        const int exitCode = RunMeasuredCase(name, settings, function);
        std::exit(exitCode);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        std::cerr << "waitpid failed for " << name << "\n";
        return 1;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        std::cerr << name << " failed";
        if (WIFSIGNALED(status)) {
            std::cerr << " by signal " << WTERMSIG(status);
        }
        std::cerr << "\n";
        return 1;
    }

    return 0;
}

static void PrintUsage(const char *programName) {
    std::cout << "Usage: " << programName
              << " [rows cols cellWidth cellHeight minScale maxScale seed1 seed2]\n";
}

static TestSettings ParseSettings(int argc, char **argv) {
    TestSettings settings{2000, 1500, 10, 10, 0.2, 0.8, 42, 43};

    if (argc == 1) {
        return settings;
    }

    if (argc != 9) {
        PrintUsage(argv[0]);
        std::exit(1);
    }

    settings.rows = std::atoi(argv[1]);
    settings.cols = std::atoi(argv[2]);
    settings.cellWidth = std::atoi(argv[3]);
    settings.cellHeight = std::atoi(argv[4]);
    settings.minScale = std::atof(argv[5]);
    settings.maxScale = std::atof(argv[6]);
    settings.seed1 = static_cast<ptrdiff_t>(std::atoll(argv[7]));
    settings.seed2 = static_cast<ptrdiff_t>(std::atoll(argv[8]));

    return settings;
}

int main(int argc, char **argv) {
    const TestSettings settings = ParseSettings(argc, argv);

    std::cout << "Memory benchmark uses separate child process.\n";
    std::cout << "Peak memory is read from /proc/self/status VmHWM.\n";
    std::cout << std::flush;

    int result = 0;
    result |= RunChild("withEdge", settings, withEdge);
    return result;
}
