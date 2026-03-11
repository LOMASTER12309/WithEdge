#ifndef GKERNEL_LABELS_HPP
#define GKERNEL_LABELS_HPP

#include <unordered_map>
#include <unordered_set>

#include "base.hpp"

namespace gkernel {

class Labels {
public:
    static Labels& getInstance() {
        static Labels instance;
        return instance;
    }

    std::unordered_set<labelType> getLabels(const IDObject& collection);

    std::vector<labelDataType>& getLabel(const IDObject& collection, const labelType& labelName);
    void createLabel(const IDObject& collection, const labelType& labelName);
    void removeLabel(const IDObject& collection, const labelType& labelName);
    void removeLabels(const IDObject& collection);
    void copyLabels(const IDObject& from, const IDObject& to);

    // Delete move/copy constructor and assignment operator
    Labels(const Labels&) = delete;
    Labels(Labels&&) = delete;
    Labels& operator=(const Labels&) = delete;
    Labels& operator=(Labels&&) = delete;

    virtual ~Labels() = default;

private:
    Labels() = default; // Private constructor for singleton
    std::unordered_map<size_t, std::unordered_map<labelType, std::vector<labelDataType>>> labels;
};

namespace labels {

const std::string contourID = "contour_id";
const std::string layerID = "layer_id";
const std::string topNeighbour = "top_neighbour";
const std::string bottomNeighbour = "bottom_neighbour";
const std::string firstLayerTop = "first_contours_layer_top";
const std::string secondLayerTop = "second_contours_layer_top";
const std::string firstLayerBottom = "first_contours_layer_bottom";
const std::string secondLayerBottom = "second_contours_layer_bottom";
const std::string contourParent = "contourParent"; // для contourLayer
const std::string contourInner = "contourInner";   // метка вложенности контуров. Для contourLayer
const std::string coincidence =
    "coincidence"; // метка наложения на ребро другого слоя. Для edgeLayer
const std::string singularityPoint = "singularityPoint"; // для contourLayer

} // namespace labels

} // namespace gkernel

#endif // GKERNEL_LABELS_HPP
