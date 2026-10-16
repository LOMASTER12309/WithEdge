#ifndef GKERNEL_BASE_HPP
#define GKERNEL_BASE_HPP

#include <array>
#include <cmath>
#include <glog/logging.h>
#include <string>
#include <utility>
#include <vector>

#include "types.hpp"

namespace gkernel {

namespace constants {

constexpr double eps = 1e-5;

} // namespace constants

static inline bool epsEqual(double a, double b) {
    return std::abs(a - b) < constants::eps;
}

struct GKernelManager {
    static bool isNeedLog;
};

class IDObject {
public:
    IDObject() : currentID(globalCounter++) {}

    IDObject(const IDObject&) = delete;

    std::size_t id() const {
        return currentID;
    }

    virtual std::size_t size() const = 0;

protected:
    std::size_t currentID;

public:
    static std::size_t globalCounter;
};

class Point {
public:
    coordType x, y;

    Point() : x(0), y(0) {
        if (GKernelManager::isNeedLog) {
            LOG(INFO) << "Point (0,0) create";
        }
    }

    Point(coordType X, coordType Y) : x(X), y(Y) {
        if (GKernelManager::isNeedLog) {
            LOG(INFO) << "Point (" + std::to_string(X) + "," + std::to_string(Y) + ") create";
        }
    }

    bool operator==(const Point& other) const {
        return this->x == other.x && this->y == other.y;
    }

    bool operator!=(const Point& other) const {
        return this->x != other.x || this->y != other.y;
    }

    bool operator<(const Point& other) const {
        return this->x < other.x || (this->x == other.x && this->y < other.y);
    }

    bool operator>(const Point& other) const {
        return this->x > other.x || (this->x == other.x && this->y > other.y);
    }

    double distanceToPoint(const Point pt) const;

    double distanceToPoint2(const Point pt) const;

    Point round() const {
        return Point(std::round(x), std::round(y));
    }
};

class Rect {
public:
    Rect(const coordType xMax, const coordType yMax, const coordType xMin, const coordType yMin)
        : minPoint(xMin, yMin), maxPoint(xMax, yMax) {}

    const Point& getPointMin() const {
        return minPoint;
    }

    const Point& getPointMax() const {
        return maxPoint;
    }

private:
    Point minPoint, maxPoint;
};

class Edge {
public:
    Edge() {
        if (GKernelManager::isNeedLog) {
            LOG(INFO) << "Edge create";
        }
    }

    Edge(const Point& p1, const Point& p2);

    Edge(const Point& p1, const Point& p2, int64_t firstParent, int64_t secondParent);

    Edge(const Edge& other);

    Edge& operator=(const Edge& other);

    bool operator==(const Edge& other) const {
        return beginPoint == other.beginPoint && endPoint == other.endPoint;
    }

    bool operator!=(const Edge& other) const {
        return beginPoint != other.beginPoint || endPoint != other.endPoint;
    }

    bool operator<(const Edge& other) const {
        return beginPoint < other.beginPoint ||
               beginPoint == other.beginPoint && endPoint < other.endPoint;
    }

    const Point& getBeginPoint() const {
        return beginPoint;
    }

    const Point& getEndPoint() const {
        return endPoint;
    }

    bool isPoint() const {
        return beginPoint == endPoint;
    }

    bool isVertical() const {
        return epsEqual(beginPoint.x, endPoint.x);
    }

    int64_t getFirstParent() const {
        return parents[0];
    }

    int64_t getSecondParent() const {
        return parents[1];
    }

    void setFirstParent(int64_t firstParent) {
        parents[0] = firstParent;
    }

    void setSecondParent(int64_t secondParent) {
        parents[1] = secondParent;
    }

    void setParent(int64_t parent, size_t index) {
        parents[index] = parent;
    }

    Point& min() {
        return *minPoint;
    }

    Point& max() {
        return *maxPoint;
    }

    const Point& min() const {
        return *minPoint;
    }

    const Point& max() const {
        return *maxPoint;
    }

    void rotate();

    void reverse() {
        std::swap(beginPoint, endPoint);
    }

    Edge getReverse() const {
        return Edge(endPoint, beginPoint);
    }

    Rect bounds() const;

    double length() const;

    double distanceMinToEdge(const Edge& edge) const;

    double distanceMinToEdge2(const Edge& edge) const;

    double distanceMidToEdge(const Edge& edge) const;

    double distanceMidToEdge2(const Edge& edge) const;

    //-pi to pi
    double angleToEdge(const Edge& edge) const;

    bool sameIncline(const Edge& edge, double eps) const;

    Edge normalized() const;

    std::string getLogInfo();

    Edge round() const {
        return Edge(beginPoint.round(), endPoint.round());
    }

private:
    Point beginPoint, endPoint;

    std::array<int64_t, 2> parents = {-1, -1};

    Point *minPoint = nullptr, *maxPoint = nullptr;

    void updateMinMax();
};

struct PointHash {
    size_t operator()(const gkernel::Point& p) const {
        return std::hash<double>()(p.x) ^ (std::hash<double>()(p.y) << 1);
    }
};

struct EdgeHash {
    size_t operator()(const gkernel::Edge& e) const {
        auto norm = e.normalized();
        PointHash pointHash;
        return pointHash(norm.getBeginPoint()) ^ (pointHash(norm.getEndPoint()) << 1);
    }
};

class EdgeSet;

class Layer : public IDObject {
public:
    virtual EdgeSet toEdgeSet() const = 0;

    virtual Rect bounds() const = 0;

    virtual ~Layer() = default;
};

class EdgeSet : public Layer {
protected:
    std::vector<Edge> edges;

public:
    EdgeSet() = default;

    EdgeSet(const std::vector<Edge>& edges) {
        this->edges = edges;
    }

    EdgeSet(EdgeSet&& other) : edges(std::move(other.edges)) {
        this->currentID = other.currentID;
    }

    EdgeSet(const EdgeSet& other) : edges(other.edges) {}

    const Edge& operator[](size_t idx) const {
        return edges[idx];
    }

    // Итераторы для foreach
    auto begin() -> decltype(edges.begin()) {
        return edges.begin();
    }

    auto end() -> decltype(edges.end()) {
        return edges.end();
    }

    // Константные итераторы
    auto begin() const -> decltype(edges.begin()) {
        return edges.begin();
    }

    auto end() const -> decltype(edges.end()) {
        return edges.end();
    }

    size_t size() const {
        return edges.size();
    }

    Rect bounds() const override;

    bool contains(const Edge& segment) const;

    void rotate();

    EdgeSet toEdgeSet() const override {
        return *this;
    }

    std::vector<Edge> getEdges() {
        return edges;
    }
};

class EdgeClusterLayer : public Layer {
private:
    std::vector<std::vector<Edge>> edges;

public:
    EdgeClusterLayer() = default;

    EdgeClusterLayer(std::vector<std::vector<Edge>>& edges) : edges(std::move(edges)) {}

    EdgeClusterLayer(EdgeClusterLayer& layer) : edges(std::move(layer.edges)) {}

    const std::vector<Edge>& operator[](size_t idx) const {
        return edges[idx];
    }

    // Итераторы для foreach
    auto begin() -> decltype(edges.begin()) {
        return edges.begin();
    }

    auto end() -> decltype(edges.end()) {
        return edges.end();
    }

    // Константные итераторы
    auto begin() const -> decltype(edges.begin()) {
        return edges.begin();
    }

    auto end() const -> decltype(edges.end()) {
        return edges.end();
    }

    size_t size() const {
        return edges.size();
    }

    EdgeSet toEdgeSet() const override {
        // TODO: заполнить нормально
        return EdgeSet();
    }

    virtual Rect bounds() const override {
        // TODO: заполнить нормально
        return Rect(0, 0, 0, 0);
    }
};

class Chain : public IDObject {
protected:
    std::vector<Edge> edges;

    std::stringstream getLogInfo();

public:
    Chain() {
        if (GKernelManager::isNeedLog) {
            LOG(INFO) << "Chain create";
        }
    }

    Chain(const std::vector<Point>& points);

    Chain(const std::vector<Edge>& chainParts) : edges(std::move(chainParts)) {
        if (GKernelManager::isNeedLog) {
            LOG(INFO) << "Chain (" << getLogInfo().str() << ") create";
        }
    }

    Chain(const Chain& other) : Chain(other.edges) {
        if (GKernelManager::isNeedLog) {
            LOG(INFO) << "Chain (" << getLogInfo().str() << +") create";
        }
    }

    const Edge& operator[](size_t idx) const {
        return edges[idx];
    }

    auto begin() -> decltype(edges.begin()) {
        return edges.begin();
    }

    auto end() -> decltype(edges.end()) {
        return edges.end();
    }

    // Константные итераторы
    auto begin() const -> decltype(edges.begin()) {
        return edges.begin();
    }

    auto end() const -> decltype(edges.end()) {
        return edges.end();
    }

    bool operator==(const Chain& other) const;

    bool operator!=(const Chain& other) const;

    std::vector<Edge>& getEdges() {
        return edges;
    }

    const Point& getBeginPoint() {
        return edges[0].getBeginPoint();
    }

    const Point& getEndPoint() {
        return edges.back().getEndPoint();
    }

    double length() const {
        double length;

        for (const auto& edge : edges) {
            length += edge.length();
        }

        return length;
    }

    size_t size() const {
        return edges.size();
    }

    void reverse();

    Rect bounds() const;

    bool contains(const Point& point) const;

    bool containsIn(const Point& point) const;

    double distanceToChain(const Chain& chain) const;

    double distanceToChain2(const Chain& chain) const;

    void addSegment(const Edge& segment) {
        edges.push_back(segment);
    }

    void insertSegment(size_t pos, const Edge& segment) {
        edges.insert(edges.begin() + pos, segment);
    }
};

class Contour : public Chain {
public:
    Contour() = default;

    Contour(const std::vector<Point>& points) : Chain(points) {
        if (GKernelManager::isNeedLog) {
            LOG(INFO) << "Contour (" << getLogInfo().str() << ") create";
        }
    }

    Contour(const std::vector<Edge>& contourParts) : Chain(contourParts) {
        if (GKernelManager::isNeedLog) {
            LOG(INFO) << "Contour (" << getLogInfo().str() << ") create";
        }
    }

    Contour(const Contour& contour) : Chain(contour.edges) {
        if (GKernelManager::isNeedLog) {
            LOG(INFO) << "Contour (" << getLogInfo().str() << ") create";
        }
    }

    bool operator==(const Contour& other) const;

    bool operator!=(const Contour& other) const;

    bool containsIn(const Point& point) const {
        return contains(point);
    }

    double distanceToContour(const Contour& contour) const {
        return distanceToChain(contour);
    }

    double distanceToContour2(const Contour& contour) const {
        return distanceToChain2(contour);
    }

    double area() const;

    double signedArea() const;

    std::vector<Contour> normalize() const;

protected:
    bool _isOuter = true;
};

class ContourSet : public Layer {
protected:
    std::vector<Contour> contours;

public:
    ContourSet() {} // временно для заглушек DRC функций

    ContourSet(const std::vector<Contour>& _contours) : contours(std::move(_contours)) {}

    ContourSet(ContourSet&& other) : contours(std::move(other.contours)) {
        currentID = other.currentID;
    }

    ContourSet(const ContourSet& other) : contours(other.contours) {}

    ContourSet& operator=(ContourSet&& other) {
        contours = std::move(other.contours);
        this->currentID = other.currentID;

        return *this;
    }

    const Contour& operator[](size_t idx) const {
        return contours[idx];
    }

    // Итераторы для foreach
    auto begin() -> decltype(contours.begin()) {
        return contours.begin();
    }

    auto end() -> decltype(contours.end()) {
        return contours.end();
    }

    // Константные итераторы
    auto begin() const -> decltype(contours.begin()) {
        return contours.begin();
    }

    auto end() const -> decltype(contours.end()) {
        return contours.end();
    }

    size_t size() const {
        return contours.size();
    }

    void addContours(std::vector<Contour>& contours) {
        this->contours.insert(this->contours.end(), std::make_move_iterator(contours.begin()),
                              std::make_move_iterator(contours.end()));
    }

    Rect bounds() const override;

    bool contains(const Contour& contour) const;

    EdgeSet merge(const ContourSet& other) const;

    EdgeSet toEdgeSet() const override;
};

using EdgeLayer = EdgeSet;
using ContourLayer = ContourSet;

} // namespace gkernel

namespace std {
template <> struct hash<gkernel::Point> {
    size_t operator()(const gkernel::Point& p) const {
        // Пример хэширования:
        size_t h1 = std::hash<decltype(p.x)>{}(p.x);
        size_t h2 = std::hash<decltype(p.y)>{}(p.y);
        return h1 ^ (h2 << 1);
    }
};
} // namespace std

#endif // GKERNEL_BASE_HPP
