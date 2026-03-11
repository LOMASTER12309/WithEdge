#ifndef CORE_EXECUTION_STATE_HPP
#define CORE_EXECUTION_STATE_HPP

#include <map>
#include <string>

#include "base.hpp"

namespace gds_reader {
class Layer;
}

namespace core {

class ExecutionState {
public:
    static void init(std::string gdsPath,
                     std::string cellName = "",
                     std::string outputPath = "",
                     bool isFlatMode = true);

    static const gkernel::Layer* getLayer(int id);

    static int addLayer(gkernel::EdgeLayer& layer, int id = -1);

    static int addLayer(gkernel::EdgeClusterLayer& layer, int id = -1);

    static int addLayer(gkernel::ContourLayer& layer, int id = -1);

    static void addResultsCount(int count) {
        resultsCount += count;
    }

    static size_t getResultsCount() {
        return resultsCount;
    }

    static void addCheckCount() {
        checksCount++;
    }

    static size_t getChecksCount() {
        return checksCount;
    }

    ~ExecutionState() {
        for (auto& [key, layer] : layers) {
            delete layer;
        }
    }

private:
    static std::map<size_t, gkernel::Layer*> layers;

    static int maxLayerId;

    static bool isLogInit;

    static int resultsCount;

    static size_t checksCount;

    static int addLayer(gkernel::Layer* layer, int id = -1);
};

} // namespace core

#endif // CORE_EXECUTION_STATE_HPP