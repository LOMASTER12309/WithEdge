#ifndef GKERNEL_INTERNAL_ALGO_SWPL_HPP
#define GKERNEL_INTERNAL_ALGO_SWPL_HPP

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <set>
#include <vector>

#include "base.hpp"
#include "internal/cross_res.hpp"
#include "internal/print_big_number.hpp"
#include "internal/timer.hpp"

namespace gkernel {
namespace internal {

static int crossCounter = 0;
static int fullCrossCounter = 0;
static int updatesCounter = 0;
static int orderedCrossCounter = 0;      // counts memory IO for segments
static int orderedCrossCounterFinal = 0; // counts real intersection math
static int insertionsCounter = 0;
static int eraseCounter = 0;
static int updateOnInsertCounter = 0;
static int updateOnEraseCounter = 0;
static size_t activeSize = 0;
static size_t activeQuery = 0;
static size_t activeSizeMax = 0;
static int eventsCompareCounter = 0;
static int allocatedItemsCounter = 0;
static size_t verticalActivesMaxSize = 0;
static size_t verticalAtXmaxSize = 0;

// #define DEBUG_PRINT
// #define TST_ACTIVE
// #define DEBUG_DUMP

template <typename ESegment>
std::array<double, 2> segmentsTouch(const ESegment& seg0, const ESegment& seg1) {
    if ((seg0.p0.x == seg1.p0.x && seg0.p0.y == seg1.p0.y) ||
        (seg0.p0.x == seg1.p1.x && seg0.p0.y == seg1.p1.y))
        return std::array<double, 2>{seg0.p0.x, seg0.p0.y};
    if ((seg0.p1.x == seg1.p0.x && seg0.p1.y == seg1.p0.y) ||
        (seg0.p1.x == seg1.p1.x && seg0.p1.y == seg1.p1.y))
        return std::array<double, 2>{seg0.p1.x, seg0.p1.y};
    return std::array<double, 2>{std::numeric_limits<double>::quiet_NaN(),
                                 std::numeric_limits<double>::quiet_NaN()};
}

template <typename ESegment>
std::array<double, 4> orderedCross(const ESegment& seg0, const ESegment& seg1) {
    orderedCrossCounter++;
    auto res = &seg0 < &seg1 ? cross(seg0, seg1) : cross(seg1, seg0);
    if (res[0] == res[0])
        orderedCrossCounterFinal++;
    return res;
}

// dummy allocator that just eats more and more memory by big chunks
// the whole point is to quicly allocate a chunk and the delete all of the cunks very fast
// suitable for ctor/dtor complex structures like set that never deallocate (i.e. does not call
// erase just inserts)
template <class T> struct AllocOnlyAllocator {
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;

    template <typename U>
    AllocOnlyAllocator(const AllocOnlyAllocator<U>& other) : chunkSize(other.chunkSize) {
        chunks.push_back(std::vector<char>(chunkSize));
    } // why the heck it is needed I do not know

    // somehow std::set creates allocator for node from allocator for key and during this process in
    // copies moves and does the above 'reinterpret' multiple times, the entity ctreated in this
    // constructor is actually used further

    T* allocate(size_t n, const void* hint = 0) {
        allocatedItemsCounter += n;

        if (n * sizeof(T) + index > chunkSize) {
            chunks.push_back(std::vector<char>(chunkSize));
            index = 0;
        }
        T* ptr = reinterpret_cast<T*>(&chunks.back()[index]);
        index += n * sizeof(T);
        return ptr;
    }

    void deallocate(T*, size_t n) {
        // does nothing, which is actually the whole point of this structure
    }

    AllocOnlyAllocator() = delete;

    AllocOnlyAllocator(size_t _chunkSize) : chunkSize(_chunkSize * sizeof(T)) {}

    void construct(T* p, const T& value) {
        _construct(p, value);
    }

    void destroy(T* p) {}

    const size_t chunkSize{};
    size_t index{};
    std::list<std::vector<char>> chunks;
};

template <typename ESegment>
std::array<double, 4> overlapVerticals(const ESegment& seg0, const ESegment& seg1) {
    std::array<double, 4> res{
        std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
    int count = 0;
    auto addIfInBetween = [&](const Point& p, const Point& p0, const Point& p1) {
        if (count == 4)
            return;
        if (p.y <= std::max(p0.y, p1.y) && p.y >= std::min(p0.y, p1.y)) {
            if (count == 2 && p.x == res[0] && p.y == res[1]) // already added
                return;
            res[count] = p.x;
            res[count + 1] = p.y;
            count += 2;
        }
    };
    addIfInBetween(seg1.p0, seg0.p0, seg0.p1);
    addIfInBetween(seg1.p1, seg0.p0, seg0.p1);
    addIfInBetween(seg0.p0, seg1.p0, seg1.p1);
    addIfInBetween(seg0.p1, seg1.p0, seg1.p1);
    if (res[1] > res[3]) {
        std::swap(res[0], res[2]);
        std::swap(res[1], res[3]);
    }

    if (res[0] == res[2] && res[1] == res[3])
        res = {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(),
               std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};

    return res;
}

template <typename ESegment>
std::array<double, 4> cross(const ESegment& seg0, const ESegment& seg1) {
    crossCounter++;
    std::array<double, 4> res{
        std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
    const double seg0Xmax = std::max(seg0.p0.x, seg0.p1.x);
    const double seg0Xmin = std::min(seg0.p0.x, seg0.p1.x);
    const double seg1Xmax = std::max(seg1.p0.x, seg1.p1.x);
    const double seg1Xmin = std::min(seg1.p0.x, seg1.p1.x);

    const double seg0Ymax = std::max(seg0.p0.y, seg0.p1.y);
    const double seg0Ymin = std::min(seg0.p0.y, seg0.p1.y);
    const double seg1Ymax = std::max(seg1.p0.y, seg1.p1.y);
    const double seg1Ymin = std::min(seg1.p0.y, seg1.p1.y);

    // compare bb-s
    if (seg0Xmax < seg1Xmin || seg1Xmax < seg0Xmin || seg0Ymax < seg1Ymin || seg1Ymax < seg0Ymin)
        return res;

    // they can cross...
    // s0.p1 * t0 + (1-t0) * s0.p0 = s1.p1 * t1 + (1-t1) * s1.p0

    // t0 * (seg0Xmax - seg0Xmin) + seg0Xmin = t1 * (seg1Xmax - seg1Xmin) + seg1Xmin
    // t0 * (seg0Ymax - seg0Ymin) + seg0Ymin = t1 * (seg1Ymax - seg1Ymin) + seg1Ymin

    // t0 * (seg0Xmax - seg0Xmin) - t1 * (seg1Xmax - seg1Xmin) =  seg1Xmin - seg0Xmin
    // t0 * (seg0Ymax - seg0Ymin) - t1 * (seg1Ymax - seg1Ymin) =  seg1Ymin - seg0Ymin

    const double dSeg0X = seg0.p1.x - seg0.p0.x;
    const double dSeg1X = seg1.p1.x - seg1.p0.x;
    const double dSeg0Y = seg0.p1.y - seg0.p0.y;
    const double dSeg1Y = seg1.p1.y - seg1.p0.y;

    const double dSegX = seg1.p1.x - seg0.p0.x;
    const double dSegY = seg1.p1.y - seg0.p0.y;

    // t0 * dSeg0X - t1 * dSeg1X =  dSegX
    // t0 * dSeg0Y - t1 * dSeg1Y =  dSegY

    // t0 * dSeg0X * dSeg1Y - t1 * dSeg1X * dSeg1Y = dSegX * dSeg1Y
    // t0 * dSeg0Y * dSeg1X - t1 * dSeg1Y * dSeg1X = dSegY * dSeg1X

    // t0 (dSeg0X * dSeg1Y - dSeg0Y * dSeg1X) = dSegX * dSeg1Y - dSegY * dSeg1X
    // t1 (dSeg0X * dSeg1Y - dSeg0Y * dSeg1X) = dSegX * dSeg0Y - dSegY * dSeg0X

    double det = dSeg0X * dSeg1Y - dSeg0Y * dSeg1X;
    double preT0 = dSegX * dSeg1Y - dSegY * dSeg1X;
    double preT1 = dSegY * dSeg0X - dSegX * dSeg0Y;
#ifdef DEBUG_PRINT
    std::cout << "det=" << det << '\n';
#endif

    if (epsEqual(det, .0)) {
#ifdef DEBUG_PRINT
        std::cout << "preT0=" << preT0 << '\n';
#endif
        if (epsEqual(preT0, .0)) { // overlap
            fullCrossCounter++;
            if (std::abs(dSeg0X) > std::abs(dSeg0Y)) { // tends horizontal
                int count = 0;
                auto addIfInBetween = [&](const Point& p, const Point& p0, const Point& p1) {
                    if (count == 4)
                        return;
                    if (p.x <= std::max(p0.x, p1.x) && p.x >= std::min(p0.x, p1.x)) {
                        if (count == 2 && p.x == res[0] && p.y == res[1]) // already added
                            return;
                        res[count] = p.x;
                        res[count + 1] = p.y;
                        count += 2;
                    }
                };
                addIfInBetween(seg1.p0, seg0.p0, seg0.p1);
                addIfInBetween(seg1.p1, seg0.p0, seg0.p1);
                addIfInBetween(seg0.p0, seg1.p0, seg1.p1);
                addIfInBetween(seg0.p1, seg1.p0, seg1.p1);
                if (res[0] > res[2]) {
                    std::swap(res[0], res[2]);
                    std::swap(res[1], res[3]);
                }
                if (res[0] != res[0] || res[1] != res[1] || res[2] != res[2] || res[3] != res[3]) {
#ifdef DEBUG_PRINT
                    std::cout << seg0.p0 << '-' << seg0.p1 << " x " << seg1.p0 << '-' << seg1.p1
                              << &seg0 << ' ' << &seg1 << " res: " << res[0] << ' ' << res[1] << ' '
                              << res[2] << ' ' << res[3] << '\n';
#endif
                    res = {std::numeric_limits<double>::quiet_NaN(),
                           std::numeric_limits<double>::quiet_NaN(),
                           std::numeric_limits<double>::quiet_NaN(),
                           std::numeric_limits<double>::quiet_NaN()};
                }
            } else { // tends vertical
                int count = 0;
                auto addIfInBetween = [&](const Point& p, const Point& p0, const Point& p1) {
                    if (count == 4)
                        return;
                    if (p.y <= std::max(p0.y, p1.y) && p.y >= std::min(p0.y, p1.y)) {
                        if (count == 2 && p.x == res[0] && p.y == res[1]) // already added
                            return;
                        res[count] = p.x;
                        res[count + 1] = p.y;
                        count += 2;
                    }
                };
                addIfInBetween(seg1.p0, seg0.p0, seg0.p1);
                addIfInBetween(seg1.p1, seg0.p0, seg0.p1);
                addIfInBetween(seg0.p0, seg1.p0, seg1.p1);
                addIfInBetween(seg0.p1, seg1.p0, seg1.p1);
                if (res[1] > res[3]) {
                    std::swap(res[0], res[2]);
                    std::swap(res[1], res[3]);
                }
                if (res[0] != res[0] || res[1] != res[1] || res[2] != res[2] || res[3] != res[3]) {
#ifdef DEBUG_PRINT
                    std::cout << seg0.p0 << '-' << seg0.p1 << " x " << seg1.p0 << '-' << seg1.p1
                              << " res: " << res[0] << ' ' << res[1] << ' ' << res[2] << ' '
                              << res[3] << '\n';
#endif
                    res = {std::numeric_limits<double>::quiet_NaN(),
                           std::numeric_limits<double>::quiet_NaN(),
                           std::numeric_limits<double>::quiet_NaN(),
                           std::numeric_limits<double>::quiet_NaN()};
                }
            }
        }

        if (res[0] == res[2] && res[1] == res[3])
            res = {
                std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(),
                std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
        return res;
    }

    if (det * preT0 < 0 || det * preT0 > det * det || det * preT1 < 0 || det * preT1 > det * det)
        return res;

    double t0 = preT0 / det;

    res[0] = (seg0.p1.x - seg0.p0.x) * t0 + seg0.p0.x;
    res[1] = (seg0.p1.y - seg0.p0.y) * t0 + seg0.p0.y;
    fullCrossCounter++;
    return res;
}

enum eGETY_TYPE { LEFT, RIGHT, MIDDLE, UNKNOWN };

// bubbleSort sort for small vectorsof ESegments crossing at same y in current x (sweepX)
template <typename T, class C, typename F>
void bubbleSort(std::vector<T>& in, const C& c, F&& custom_swap) {
    int n = in.size();
    for (int i = 0; i < n - 1; i++) {
        int imin = i;
        for (int j = i + 1; j < n; j++) {
            imin = c(in[j], in[imin]) ? j : imin;
        }
        if (i == imin)
            continue;
        custom_swap(in[imin], in[i]);
        std::swap(in[imin], in[i]);
    }
}

// this function retreives precise y for x, instead of calculating cross point between segment(T)
// line and vertical line at x it retrives exact y coordinates of edge points of segment if x
// perfectly matches x coordinates of edge points it also report result type: left, right, middle,
// thus in cases of 'left' and 'right' result is precise
template <typename T> void getYprecise(double sweepX, const T& seg, double& y, eGETY_TYPE& type) {
    const bool inverted = seg->p0.x > seg->p1.x;
    const double xmin = inverted ? seg->p1.x : seg->p0.x;
    const double xmax = inverted ? seg->p0.x : seg->p1.x;
    if (epsEqual(xmin, sweepX)) {
        y = inverted ? seg->p1.y : seg->p0.y;
        type = LEFT;
    } else if (epsEqual(xmax, sweepX)) {
        y = inverted ? seg->p0.y : seg->p1.y;
        type = RIGHT;
    } else {
        y = seg->getY(sweepX);
        type = MIDDLE;
    }
}

// compare segments with precise y values at the segment endpoints
// accounts for segment tilt in comparison when y values are the same
// thus introducing prper order after shared point of cross point
template <typename T> struct ActiveItemsComparator {
    ActiveItemsComparator(double& _sweepX) : sweeepX(_sweepX) {}

    double& sweeepX;

    constexpr bool operator()(const T& a, const T& b) const {
        double aSweepY, bSweepY;
        eGETY_TYPE aType = eGETY_TYPE::UNKNOWN;
        eGETY_TYPE bType = eGETY_TYPE::UNKNOWN;

        getYprecise(sweeepX, a, aSweepY, aType);
        getYprecise(sweeepX, b, bSweepY, bType);

        if (!epsEqual(aSweepY, bSweepY)) {
            return aSweepY < bSweepY;
        }

        double aK = a->getK();
        double bK = b->getK();

        if (epsEqual(aK, bK)) {
            return a < b;
        }

        return aK < bK;
    }
};

template <typename E> struct EventsComparator {
    constexpr bool operator()(const E& e0, const E& e1) const {
        eventsCompareCounter++;
        // compare values, or ordered pointer pairs if values are equal
        if (e0.val != e1.val)
            return e0.val < e1.val;
        else {
            bool e0Crossing = e0.isCrossing();
            bool e1Crossing = e1.isCrossing();
            if (e0Crossing && e1Crossing)
                return e0.crossingId < e1.crossingId;
            else {
                if (e0Crossing == e1Crossing)
                    return std::max(e0.seg0, e0.seg1) == std::max(e1.seg0, e1.seg1)
                               ? std::min(e0.seg0, e0.seg1) < std::min(e1.seg0, e1.seg1)
                               : std::max(e0.seg0, e0.seg1) < std::max(e1.seg0, e1.seg1);
                else
                    return e0Crossing < e1Crossing;
            }
        }
        // order:
        // first go an events with smaller x
        //   then among events at same x first go non crossing events
        //     among non crossing events start events go first
        //   then go crossing events
        //     among crossing events first goes event with less crossingId
    }
};

template <typename S, typename R> struct VerticalIntersector {
    struct VerticalActiveItem {
        VerticalActiveItem(double _y, int _id, bool _start) : y(_y), id(_id), start(_start) {}

        double y;
        int id;
        bool start;
    };

    VerticalIntersector(R& _res) : res(&_res) {}

    R* res;
    std::vector<int> actives;
    std::vector<VerticalActiveItem> events;

    void run(std::vector<S*> segments) {
        verticalAtXmaxSize = std::max(verticalAtXmaxSize, segments.size());
        actives.clear();
        events.clear();
        for (int segId = 0; segId < (int)segments.size(); segId++) {
            auto seg = segments[segId];
            bool inv = seg->p0.y > seg->p1.y;
            events.push_back(VerticalActiveItem(seg->p0.y, segId, !inv));
            events.push_back(VerticalActiveItem(seg->p1.y, segId, inv));
        }
        std::sort(
            events.begin(), events.end(),
            [](const VerticalActiveItem& a, const VerticalActiveItem& b) { return a.y < b.y; });
        for (auto& event : events) {
            if (event.start) { // starting event
                // intersect with all others present in actives
                for (auto& active : actives) {
                    if (active == -1)
                        continue;
                    const auto& seg0 = *segments[event.id];
                    const auto& seg1 = *segments[active];
                    auto overlapCoords = overlapVerticals(seg0, seg1);
                    if (overlapCoords[2] == overlapCoords[2]) {
                        CrossOverlap overlap(overlapCoords[0], overlapCoords[1], overlapCoords[2],
                                             overlapCoords[3]);
                        overlap.addEdge(seg0.origSeg);
                        overlap.addEdge(seg1.origSeg);
                        res->overlaps.emplace_back(overlap);
                    }
                }
                // manage actives:
                bool found = false;
                for (auto& active : actives) {
                    if (active == -1) {
                        active = event.id;
                        found = true;
                    }
                }
                if (!found) // no empty place in actives so expand it
                    actives.push_back(event.id);
                verticalActivesMaxSize = std::max(verticalActivesMaxSize, actives.size());
            } else {
                for (auto& active : actives) {
                    if (event.start)
                        continue;
                    if (active == event.id) {
                        active = -1;
                        break;
                    }
                }
            }
        }
    }
};

template <typename TOriginalSegment, typename TOrigSegContainer>
CrossResVec SweepLine(const TOrigSegContainer& _segs) {
    CrossResVec res;

    double sweepX; // we use vertical sweep line, sweepX is an x coordinate of the line

    struct ESegment;

    typedef std::set<ESegment*, ActiveItemsComparator<ESegment*>> Active;

    struct ESegment {
        ESegment(const Point& _p0, const Point& _p1) : p0(_p0), p1(_p1) {}

        Point p0, p1;
        typename Active::iterator activeIt;
        const TOriginalSegment* origSeg{};
        bool ending{};
#ifdef DEBUG_PRINT
        int id;
#endif

        double getY(double x) const {
            updatesCounter++;
            // return (p0.y * (p1.x - x) + p1.y * (x - p0.x) ) / (p1.x - p0.x); //NOTE: this gives
            // different y for two horizontal lines at the same height
            return (p0.y * p1.x - p1.y * p0.x + x * (p1.y - p0.y)) /
                   (p1.x - p0.x); // and this one gives the same!
        }

        double getK() const {
            return (p1.y - p0.y) / (p1.x - p0.x);
        }

        double getRight() const {
            return std::max(p0.x, p1.x);
        }

        double getLeft() const {
            return std::min(p0.x, p1.x);
        }

        double getBottom() const {
            return std::min(p0.y, p1.y);
        }

        double getTop() const {
            return std::max(p0.y, p1.y);
        }

        void show() {
            std::cout << p0.x << " " << p0.y << " " << p1.x << " " << p1.y << std::endl;
        }
    };

    [[maybe_unused]] auto ID = [](const ESegment* es) {
#ifdef DEBUG_PRINT
        return (void*)(es ? es->id : 0);
#else
        return es;
#endif
    };

    auto swapESegments = [](ESegment* e0, ESegment* e1) {
        std::swap(e0->activeIt, e1->activeIt);
        std::swap((ESegment*&)*e0->activeIt, (ESegment*&)*e1->activeIt);
    };

    struct Event {
        enum eTYPE { BEGIN, END, CROSS, UNKNOWN };

        Event(ESegment* _a, ESegment* _b, double _val, int crossId = -1)
            : seg0(_a), seg1(_b), val(_val), crossingId(crossId) {}

        ESegment* seg0{};
        ESegment* seg1{};
        double val;
        int crossingId = -1; // filled in from current crossing counter to distinguish crossings in
                             // order they appear

        bool isCrossing() const {
            return seg0 && seg1 && (seg0 != seg1);
        }

        bool isVertical() const {
            return seg0 == seg1;
        }

        bool isBegin() const {
            return seg0 && !seg1;
        }

        bool isEnd() const {
            return !seg0 && seg1;
        }

        bool isEqual(
            const Event& other) const { // NOTE: crossingId does not count for event equality!
            return epsEqual(val, other.val) &&
                   std::max(seg0, seg1) == std::max(other.seg0, other.seg1) &&
                   std::min(seg0, seg1) == std::min(other.seg0, other.seg1);
        }

        eTYPE getType() const {
            if (seg0 && seg1)
                return CROSS;
            else if (seg0 && !seg1)
                return BEGIN;
            else if (!seg0 && seg1)
                return END;
            else
                return UNKNOWN;
        }

        double getY(double x) const {
            eGETY_TYPE t;
            double y;
            getYprecise(x, seg0 ? seg0 : seg1, y, t);
            return y;
        }
    };

    // this structure keeps some internal vectors
    // main method (run) implements fast overlap of
    // collinear segments at same X
    VerticalIntersector<ESegment, CrossResVec> verticalIntersector(res);

    ActiveItemsComparator<ESegment*> activeItemsComparator(sweepX);

    Active active(activeItemsComparator);
#ifdef TST_ACTIVE
    Active activeTest(activeItemsComparator);
#endif

    timer tim("hi!");
    updatesCounter = 0;
    orderedCrossCounter = 0;
    orderedCrossCounterFinal = 0;

    int failures = 0;

    std::vector<ESegment> segments;
    segments.reserve(_segs.size());
#ifdef DUMPERRORCASE
    std::ofstream ofsegs("error.dat", std::ios_base::binary);
#endif
    for (size_t i = 0; i < _segs.size(); i++) {
        auto& segment = _segs[i];
#ifdef DUMPERRORCASE
        double s[4];
        s[0] = segment.getBeginPoint().x;
        s[1] = segment.getBeginPoint().y;
        s[2] = segment.getEndPoint().x;
        s[3] = segment.getEndPoint().y;
        ofsegs.write((char*)s, 4 * sizeof(double));
#endif
        segments.emplace_back(ESegment(segment.getBeginPoint(), segment.getEndPoint()));
        segments.back().origSeg = &segment;
        segments.back().activeIt = active.end();
        // segments.back().show();

#ifdef DEBUG_PRINT
        segments.back().id = i + 1;
#endif
    }
#ifdef DUMPERRORCASE
    ofsegs.close();
#endif
#ifdef DEBUG_PRINT
    std::cout << "doing sweepline on " << big(segments.size()) << " segments\n";
#endif
    // DUMP
#ifdef DEBUG_DUMP
    std::ofstream of("test.txt");
    for (auto& s : segments) {
        of << "lines: " << ID(&s) << '\n';
        of << s.p0 << ' ' << s.p1 << '\n';
    }
    of << "points: p\n";
    for (auto& s : segments) {
        of << s.p0 << '\n';
        of << s.p1 << '\n';
    }
    of << "lines: g\n";
    for (auto& s : segments) {
        of << s.p0 << ' ' << s.p1 << " (.5,.5,.5)\n";
    }
    of.close();
#endif

    EventsComparator<Event> eventsComparator;

    std::vector<Event> eventsVec;
    eventsVec.reserve(segments.size() * 2);

#ifdef DEBUG_PRINT
    std::cout << "esegment construct time is " << tim.elapsed() << '\n';
#endif
    for (auto& segment : segments) {
        if (!epsEqual(segment.p0.x, segment.p1.x)) {
            eventsVec.emplace_back(Event(&segment, nullptr, std::min(segment.p0.x, segment.p1.x)));
            eventsVec.emplace_back(Event(nullptr, &segment, std::max(segment.p0.x, segment.p1.x)));
        } else
            eventsVec.emplace_back(Event(&segment, &segment, segment.p0.x));
    }
#ifdef DEBUG_PRINT
    std::cout << "+build vector time is " << tim.elapsed() << '\n';
    std::cout << "eventsCompareCounter: " << big(eventsCompareCounter) << '\n';
#endif
    std::sort(eventsVec.begin(), eventsVec.end(),
              [eventsComparator](const Event& ev0, const Event& ev1) {
                  return eventsComparator(ev0, ev1);
              });

#ifdef DEBUG_PRINT
    std::cout << "+build events sort time is " << tim.elapsed() << '\n';
    std::cout << "eventsCompareCounter: " << big(eventsCompareCounter) << '\n';
    std::cout << "building Events\n";
#endif
    AllocOnlyAllocator<Event> allocEvents(1024);
    std::set<Event, EventsComparator<Event>, AllocOnlyAllocator<Event>> events(
        eventsVec.begin(), eventsVec.end(), eventsComparator, allocEvents);

#ifdef DEBUG_PRINT
    for (auto ev : events) {
        std::cout << ev.val;
        for (int i = 0; i < sizeof(double); i++)
            std::cout << '|' << (int)((char*)&ev.val)[i];
        std::cout << ' ';
    }
    std::cout << '\n';
    std::cout << "+build events time is " << tim.elapsed() << '\n';
    std::cout << "eventsCompareCounter: " << big(eventsCompareCounter) << '\n';
#endif

    tim.reset();
    // left and right events are added now process them
    auto it = events.begin();
    sweepX = it->val;

    struct EventAtX {
        EventAtX(const Event* _ev, double _y) : event(_ev), y(_y) {}

        const Event* event;
        double y;

        constexpr bool operator<(const EventAtX& other) const {
            // first compare y
            if (!epsEqual(y, other.y))
                return y < other.y;
            // then types: NEW SEGMENTS COME FIRST
            const auto type = event->getType();
            const auto otherType = other.event->getType();

            if (type != otherType)
                return type < otherType;
            // then compare k
            if (type == Event::eTYPE::END) {
                const double k = event->seg1->getK();
                const double otherK = other.event->seg1->getK();
                return k > otherK;
            } else {
                const double k = event->seg0->getK();
                const double otherK = other.event->seg0->getK();
                return k < otherK;
            }
        }
    };

    std::vector<EventAtX> eventsAtCurrentX;
    std::vector<ESegment*> verticalSegmentsAtCurrentX;

    // now let us insert first (definetelly) start event to active and to activeEventsAtCurrentX
    // activeEventsAtCurrentX.push_back(ActiveElement((Event*) &(*it),
    // active.insert(ActiveItem(sweepX,it->seg0)).first, active.end()));

    int iter = 0;
    int currentCrossingId = 0;
#ifdef DEBUG_DUMP
    std::ofstream evof("events.txt");
    evof.close();
#endif

    auto intersectActives = [&](typename Active::iterator& actIt0,
                                typename Active::iterator& actIt1) {
        if (actIt0 == actIt1)
            return std::array<double, 4>{
                std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(),
                std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};

        auto crossingCoords = orderedCross(**actIt0, **actIt1);

        if (crossingCoords[2] == crossingCoords[2]) {
            CrossOverlap overlaps(crossingCoords[0], crossingCoords[1], crossingCoords[2],
                                  crossingCoords[3]);
            overlaps.addEdge((*actIt0)->origSeg);
            overlaps.addEdge((*actIt1)->origSeg);
            res.overlaps.emplace_back(overlaps);
            return crossingCoords;
        }

        if (crossingCoords[0] == crossingCoords[0]) {
#ifdef DEBUG_PRINT
            std::cout << "intersectActives : " << ID(*actIt0) << ' ' << ID(*actIt1) << " V\n";
#endif
            CrossPoint crossPt(crossingCoords[0], crossingCoords[1]);
            crossPt.addEdge((*actIt0)->origSeg);
            crossPt.addEdge((*actIt1)->origSeg);

            // merge intersection to segment endpoint if possible
            // avoid adding intersections with endpoints
            crossingCoords[0] = epsEqual(crossingCoords[0], (*actIt0)->getRight())
                                    ? (*actIt0)->getRight()
                                    : crossingCoords[0];
            crossingCoords[0] = epsEqual(crossingCoords[0], (*actIt0)->getLeft())
                                    ? (*actIt0)->getLeft()
                                    : crossingCoords[0];
            crossingCoords[0] = epsEqual(crossingCoords[0], (*actIt1)->getRight())
                                    ? (*actIt1)->getRight()
                                    : crossingCoords[0];
            crossingCoords[0] = epsEqual(crossingCoords[0], (*actIt1)->getLeft())
                                    ? (*actIt1)->getLeft()
                                    : crossingCoords[0];

            if ((crossingCoords[0] == (*actIt0)->getRight() ||
                 crossingCoords[0] == (*actIt0)->getLeft()) &&
                (crossingCoords[0] == (*actIt1)->getRight() ||
                 crossingCoords[0] == (*actIt1)->getLeft()))
                return crossingCoords;

            res.emplace_back(crossPt);

            if ((crossingCoords[0] == (*actIt0)->getRight() ||
                 crossingCoords[0] == (*actIt1)->getRight()) ||
                (crossingCoords[0] == (*actIt0)->getLeft() ||
                 crossingCoords[0] == (*actIt1)->getLeft()))
                return crossingCoords; // do not add shared point to events. it is already there,
                                       // new one added as cross point will make a mess

            bool inVicinity = epsEqual(crossingCoords[0], sweepX);
            if (crossingCoords[0] < sweepX && !inVicinity) // event is definetelly in the past
                return crossingCoords;

            crossingCoords[0] =
                (crossingCoords[0] < sweepX && inVicinity)
                    ? sweepX
                    : crossingCoords[0]; // do not go back in time slightly, stay at sweepX if close

            // find Event in Events in the vicinity of sweepX;
            // we have to brute force alittle because of additional parameter currentCrossingId
            // and slight difference in x (Event.val).
            auto findThisEvent = [&](const Event& candidate) -> bool {
                auto candidateTester =
                    Event(*actIt0, *actIt1, crossingCoords[0], std::numeric_limits<int>::max());
                auto possibleSibling0 = events.lower_bound(candidateTester);
                auto possibleSibling = possibleSibling0;
                while (true) {
                    if (possibleSibling == events.begin())
                        break;
                    possibleSibling--; // due to intmax lower_bound returns next event
                    if (!epsEqual(possibleSibling->val, crossingCoords[0]))
                        break;
#ifdef DEBUG_PRINT
                    std::cout << "possibleSibling.val = " << possibleSibling->val << '\n';
#endif
                    // we have a sibling for this crossing, skip adding
                    if (candidate.isEqual(*possibleSibling))
                        return true;
                }
                while (true) {
                    if (possibleSibling == events.end())
                        break;
                    possibleSibling++; // due to intmax lower_bound returns next event
                    if (!epsEqual(possibleSibling->val, crossingCoords[0]))
                        break;
#ifdef DEBUG_PRINT
                    std::cout << "possibleSibling.val = " << possibleSibling->val << '\n';
#endif
                    // we have a sibling for this crossing, skip adding
                    if (candidate.isEqual(*possibleSibling))
                        return true;
                }
                return false;
            };

            // avoid adding intersections already added
            auto candidate = Event(*actIt0, *actIt1, crossingCoords[0], currentCrossingId);

            if (findThisEvent(candidate))
                return crossingCoords;

#ifdef DEBUG_PRINT
            std::cout << "ADDING INTERSECTION EVENT at " << crossingCoords[0] << " for "
                      << ID(*actIt0) << ", " << ID(*actIt1) << '\n';
#endif
            events.insert(candidate);
#ifdef DEBUG_PRINT
            for (auto event : events)
                std::cout << event.val << ':' << event.crossingId << ':' << ID(event.seg0) << ':'
                          << ID(event.seg1) << ' ';
            std::cout << '\n';
#endif
            currentCrossingId++;
        } else {
#ifdef DEBUG_PRINT
            std::cout << "intersectActives : " << ID(*actIt0) << ' ' << ID(*actIt1) << " X\n";
#endif
        }
        return crossingCoords;
    };

    auto intersectNeighborsWithMe = [&](typename Active::iterator& actIt) {
        std::array<std::array<double, 4>, 2> res;
        auto next = actIt;
        next++;
        if (next != active.end())
            res[1] = intersectActives(actIt, next);

        if (actIt != active.begin()) {
            auto prev = actIt;
            prev--;
            res[0] = intersectActives(prev, actIt);
        }
        return res;
    };

    auto intersectWithNeighborsAtSameY = [&](typename Active::iterator& actIt, double y) {
        auto neighborIt = actIt;

        double neighborY = y;
        // forward traverse over non-ending segmens at this y
        while (true) {
            neighborIt++;
            if (neighborIt != active.end()) {
                if ((*neighborIt)->ending)
                    continue;
                neighborY = (*neighborIt)->getY(sweepX);
                if (!epsEqual(neighborY, y))
                    break;
                intersectActives(actIt, neighborIt);
            } else
                break;
        }
        // backward traverse over non-ending segmens at this y
        neighborY = y;
        neighborIt = actIt;
        while (true) {
            if (neighborIt != active.begin()) {
                neighborIt--;
                if ((*neighborIt)->ending)
                    continue;
                neighborY = (*neighborIt)->getY(sweepX);
                if (!epsEqual(neighborY, y))
                    break;
                intersectActives(actIt, neighborIt);
            } else
                break;
        }
    };

    while (it != events.end()) {
#ifdef DEBUG_PRINT
        std::cout << "-----------------------------------------\n";
#endif

        // we are in same X, let us just collect events for further processing:
        if (epsEqual(sweepX, it->val)) {
            // if it->val is a little bit ahead of sweepX then we need to update sweepX:
            sweepX = it->val;
            if (!(it->seg0 && it->seg1 && it->seg0 == it->seg1))
                eventsAtCurrentX.push_back(EventAtX(&*it, it->getY(sweepX)));

            if (it->seg0 && it->seg1 && it->seg0 == it->seg1) { // vertical
                verticalSegmentsAtCurrentX.push_back(it->seg0);
            }
            it++;
            if (it != events.end())
                continue;
        }
        // add overlaps of vertical segments at this x:
        // TODO: do faster approach than n^2
        /*
        for (size_t i = 0; i < verticalSegmentsAtCurrentX.size(); i++) {
            for (size_t j = i + 1; j < verticalSegmentsAtCurrentX.size(); j++) {
                auto overlapCoords = overlapVerticals(*verticalSegmentsAtCurrentX[i],
                                                      *verticalSegmentsAtCurrentX[j]);
                if (overlapCoords[2] == overlapCoords[2]) {
                    CrossOverlap overlap(overlapCoords[0], overlapCoords[1], overlapCoords[2],
                                         overlapCoords[3]);
                    overlap.addSegment(verticalSegmentsAtCurrentX[i]->origSeg);
                    overlap.addSegment(verticalSegmentsAtCurrentX[j]->origSeg);
                    res.overlaps.emplace_back(overlap);
                }
            }
        }
        */
        verticalIntersector.run(verticalSegmentsAtCurrentX);
#ifdef DEBUG_DUMP
        std::ofstream evof("events.txt", std::ios_base::app);
        evof << "lines: " << sweepX << '\n';
        evof << "(" << sweepX << ",-2,0) (" << sweepX << ",16,0) (1,0,0)\n";
        evof.close();
#endif
        it--; // step back before we added more events
#ifdef DEBUG_PRINT
        std::cout << " actives: " << active.size() << '\n';
#endif
        if (eventsAtCurrentX.size() > 1)
            std::sort(eventsAtCurrentX.begin(), eventsAtCurrentX.end());

#ifdef DEBUG_PRINT
        std::cout << " actives: " << active.size() << '\n';
#endif
#define DUMPACTIVES
#ifdef DEBUG_PRINT for (auto aaa : active) std::cout << ' ' << ID(aaa); std::cout << '\n';
#endif
#ifdef DEBUG_PRINT
        DUMPACTIVES
#endif

        // swap crossing segments
        static std::vector<EventAtX*> eventsCrossAtX;
        for (auto& ex : eventsAtCurrentX) {
            if (ex.event->getType() != Event::eTYPE::BEGIN)
                eventsCrossAtX.push_back(&ex);
        }
#ifdef DEBUG_PRINT
        std::cout << "before sort crossings:";
        for (auto* ex : eventsCrossAtX)
            std::cout << ' ' << ID(ex->event->seg0) << ' ' << ID(ex->event->seg1) << '\n';
#endif

        std::sort(eventsCrossAtX.begin(), eventsCrossAtX.end(),
                  [](const EventAtX* ea, const EventAtX* eb) {
                      if (!epsEqual(ea->y, eb->y))
                          return ea->y < eb->y;
                      auto aMax = std::max(ea->event->seg0, ea->event->seg1);
                      auto bMax = std::max(eb->event->seg0, eb->event->seg1);
                      if (aMax != bMax)
                          return aMax < bMax;
                      auto aMin = std::min(ea->event->seg0, ea->event->seg1);
                      auto bMin = std::min(eb->event->seg0, eb->event->seg1);
                      return aMin < bMin;
                  });

        {
            static std::vector<ESegment*> ends;
            for (auto ex : eventsCrossAtX) {
                if (ex->event->getType() == Event::eTYPE::END) {
                    ends.push_back(ex->event->seg1);
                    ex->event->seg1->ending = true;
                }
            }

            std::sort(ends.begin(), ends.end(), [&](const ESegment* a, const ESegment* b) {
                double aY = a->getY(sweepX);
                double bY = b->getY(sweepX);
                if (!epsEqual(aY, bY))
                    return aY < bY;
                double aK = a->getK();
                double bK = b->getK();
                if (!epsEqual(aK, bK))
                    return aK > bK; // sort in inverse order since we did not pass sweepX yet
                return a < b;
            });

#ifdef DEBUG_PRINT
            std::cout << "ends:";
            for (auto xxx : ends)
                std::cout << ' ' << ID(xxx);
            std::cout << '\n';
#endif

            auto endsHave = [&](ESegment* seg) { return seg->ending; };

            auto bottom = active.end();
            auto top = active.end();
            for (size_t i = 0; i < ends.size(); i++) {
                bool intersect = false;
#ifdef DEBUG_PRINT
                std::cout << "it=" << ID(ends[i]) << '\n';
#endif
                auto it = ends[i]->activeIt;
                if (it == active.begin())
                    continue;
                auto prev = it;
                prev--;
                auto next = it;
                next++;
                if (next == active.end())
                    continue;

#ifdef DEBUG_PRINT
                std::cout << "i=" << i << " prev=" << ID(*prev) << " next=" << ID(*next) << '\n';
#endif
                if (!endsHave(*prev)) {
#ifdef DEBUG_PRINT
                    std::cout << "BOTTOM! = " << ID(*prev) << '\n';
#endif
                    bottom = prev;
                    intersect = true;
                }

                if (!endsHave(*next)) {
#ifdef DEBUG_PRINT
                    std::cout << "TOP! = " << ID(*next) << '\n';
#endif
                    top = next;
                    intersect = true;
                }

                if (top != active.end() && bottom != active.end() && intersect)
                    intersectActives(bottom, top);
            }

            // now intersect each end with all transitional(crossing) segments at this y
            for (size_t i = 0; i < ends.size(); i++) {
                // ends[i]->show();
                auto segIt = ends[i]->activeIt;
                double y = (*segIt)->p0.x < (*segIt)->p1.x ? (*segIt)->p1.y : (*segIt)->p0.y;
                intersectWithNeighborsAtSameY(segIt, y);
            }

            ends.clear();
        }

#ifdef DEBUG_PRINT
        std::cout << "after sort crossings:";
        for (auto* ex : eventsCrossAtX)
            std::cout << ' ' << ID(ex->event->seg0) << ' ' << ID(ex->event->seg1) << '\n';
#endif

        static std::vector<ESegment*> segsAtY;
        static std::vector<ESegment*> uniqueSegsAtY;
        for (size_t exId = 0; exId < eventsCrossAtX.size(); exId++) {
            // for each y do:
            //   cross all segments in this y without setting new events
            //   reorder by swapping (bubbleSort sort)
            //     find all segments in current y by stating with first and traversing actives. use
            //     getY() to find out if the segmnet is in current y swap (getK will be used for new
            //     order)
            auto& eventAtX = eventsCrossAtX[exId];
            if (eventAtX->event->seg0)
                segsAtY.push_back(eventAtX->event->seg0);
            if (eventAtX->event->seg1)
                segsAtY.push_back(eventAtX->event->seg1);

            if (exId + 1 == eventsCrossAtX.size() ||
                !epsEqual(eventsCrossAtX[exId]->y, eventsCrossAtX[exId + 1]->y)) {
#ifdef DEBUG_PRINT
                std::cout << "segsAtY size is " << segsAtY.size() << " :";
                for (auto v : segsAtY)
                    std::cout << ' ' << ID(v);
                std::cout << '\n';
#endif
                double y = eventAtX->y;
                // now spread around first ESegment from segsAtY
                auto it0 = segsAtY[0]->activeIt;
#ifdef DEBUG_PRINT
                std::cout << "it0=" << (it0 == active.end() ? (void*)-1 : ID(*it0)) << '\n';
#endif
                auto itEnd = it0;
                while (true) {
#ifdef DEBUG_PRINT
                    std::cout << "itEnd=" << (itEnd == active.end() ? (void*)-1 : ID(*itEnd))
                              << '\n';
#endif
                    if (itEnd == active.end() || !epsEqual((*itEnd)->getY(sweepX), y))
                        break;
                    itEnd++;
                }
                auto itBegin = it0;
                while (true) {
                    if (itBegin == active.begin())
                        break;
                    itBegin--;
#ifdef DEBUG_PRINT
                    std::cout << "itBegin=" << ID(*itBegin) << '\n';
#endif
                    if (!epsEqual((*itBegin)->getY(sweepX), y)) {
                        itBegin++;
                        break;
                    }
                }
#ifdef DEBUG_PRINT
                std::cout << "itBegin=" << (itBegin == active.end() ? (void*)-1 : ID(*itBegin))
                          << " itEnd=" << (itEnd == active.end() ? (void*)-1 : ID(*itEnd)) << '\n';
#endif
                for (auto it = itBegin; it != itEnd; it++) {
                    uniqueSegsAtY.push_back((ESegment*)*it);
                }

#ifdef DEBUG_PRINT
                std::cout << "uniqueSegsAtY size is " << uniqueSegsAtY.size() << '\n';
#endif
                // swapsort
#ifdef DEBUG_PRINT
                std::cout << "bubbleSort";
                for (auto b : uniqueSegsAtY)
                    std::cout << ' ' << ID(b);
                std::cout << '\n';
#endif
                bubbleSort(uniqueSegsAtY, activeItemsComparator, swapESegments);
#ifdef DEBUG_PRINT
                std::cout << "bubbleSort";
                for (auto b : uniqueSegsAtY)
                    std::cout << ' ' << ID(b);
                std::cout << '\n';
#endif

                uniqueSegsAtY.clear();
                segsAtY.clear();
            }
        }

        eventsCrossAtX.clear();

#ifdef DEBUG_PRINT
        std::cout << " actives: " << active.size() << '\n';
#endif

        auto intersectActivesWithVertical = [&](bool onlyEnding = false) {
            // intersect all vectical segments with segments in Active
            for (auto vertSegAtX : verticalSegmentsAtCurrentX) {
                double yMin = std::min(vertSegAtX->p0.y, vertSegAtX->p1.y);
                double yMax = std::max(vertSegAtX->p0.y, vertSegAtX->p1.y);
                // note here we need to chage 1 to bb min bb max
                ESegment segMin(Point(sweepX - 1., yMin), Point(sweepX + 1., yMin));
                auto itMin = active.upper_bound(&segMin);
                if (itMin != active.begin())
                    itMin--;
                while (itMin != active.begin() && epsEqual((*itMin)->getY(sweepX), yMin))
                    itMin--;

                // note here we need to chage 1 to bb min bb max:
                ESegment segMax(Point(sweepX - 1., yMax), Point(sweepX + 1., yMax));
                auto itMax = active.upper_bound(&segMax);

                while (itMax != active.end() && epsEqual((*itMax)->getY(sweepX), yMax))
                    itMax++;

                for (auto it = itMin; it != itMax; it++) {
                    if (onlyEnding && !(*it)->ending)
                        continue;
                    auto crossingCoords = orderedCross(**it, *vertSegAtX);
                    if (crossingCoords[0] == crossingCoords[0] &&
                        crossingCoords[2] != crossingCoords[2]) {
                        CrossPoint crossPoint(crossingCoords[0], crossingCoords[1]);
                        auto seg0 = *it;
                        auto seg1 = vertSegAtX;
                        crossPoint.addEdge(seg0->origSeg);
                        crossPoint.addEdge(seg1->origSeg);

                        crossingCoords[1] = epsEqual(crossingCoords[1], seg0->getTop())
                                                ? seg0->getTop()
                                                : crossingCoords[1];
                        crossingCoords[1] = epsEqual(crossingCoords[1], seg0->getBottom())
                                                ? seg0->getBottom()
                                                : crossingCoords[1];
                        crossingCoords[1] = epsEqual(crossingCoords[1], seg1->getTop())
                                                ? seg1->getTop()
                                                : crossingCoords[1];
                        crossingCoords[1] = epsEqual(crossingCoords[1], seg1->getBottom())
                                                ? seg1->getBottom()
                                                : crossingCoords[1];

                        auto touch = segmentsTouch(*seg0, *seg1);
                        if (touch[0] != touch[0])
                            res.emplace_back(crossPoint);
                    }
                    if (crossingCoords[2] == crossingCoords[2]) {
                        auto seg0 = *it;
                        auto seg1 = vertSegAtX;
                        CrossOverlap overlap(crossingCoords[0], crossingCoords[1],
                                             crossingCoords[2], crossingCoords[3]);
                        overlap.addEdge(seg0->origSeg);
                        overlap.addEdge(seg1->origSeg);
                        res.overlaps.emplace_back(overlap);
                    }
                }
            }
        };

        intersectActivesWithVertical(true); // intersect all verticals with ending segments
        // we will do the same for transitional(crossing) segments and starting segments

#ifdef DEBUG_PRINT
        std::cout << "-------------------\nswx=" << sweepX << " actives: " << active.size()
                  << " eventsAtCurrentX.size() =" << eventsAtCurrentX.size();
        for (auto& e : eventsAtCurrentX)
            std::cout << ' ' << ID(e.event->seg0) << ':' << ID(e.event->seg1) << ':' << e.y << ':'
                      << e.event->crossingId;
        std::cout << '\n';
#endif
        {
            // just erase those who ought to be erased
            for (auto it = eventsAtCurrentX.begin(); it != eventsAtCurrentX.end(); it++) {

                if (it->event->getType() == Event::eTYPE::END) {
                    // CAUTION intersections might be there also... do something more accurate here

#ifdef DEBUG_PRINT
                    DUMPACTIVES
                    std::cout << "erasing " << ID(it->event->seg1) << ' '
                              << ID(*it->event->seg1->activeIt) << '\n'
                              << std::flush;
#endif
                    size_t size = active.size();

#ifdef TST_ACTIVE
                    for (auto itt = active.begin(); itt != active.end(); itt++) {
                        activeTest.insert(*itt);
                        std::cout << "TST: ";
                        for (auto itt = activeTest.begin(); itt != activeTest.end(); itt++)
                            std::cout << ID(*itt) << ' ';
                        std::cout << '\n';
                    }
                    if (activeTest.size() != active.size())
                        std::cout << "ERROR: sweepX" << sweepX << " TST_ACTIVE\n";
                    else {
                        auto itt = active.begin();
                        auto itt_tst = activeTest.begin();
                        for (int i = 0; i < active.size(); i++) {
                            if (*itt != *itt_tst)
                                std::cout << "ERROR: sweepX" << sweepX << " TST_ACTIVE=" << ID(*itt)
                                          << '\n';
                            itt++;
                            itt_tst++;
                        }
                    }
                    activeTest.clear();
#endif
                    active.erase(*it->event->seg1->activeIt);

#ifdef DEBUG_PRINT
                    DUMPACTIVES
#endif
                    if (active.size() + 1 != size)
#ifdef DEBUG_PRINT
                        std::cout << "ERROR! at " << sweepX << '\n';
#endif
                    it->event->seg1->activeIt = active.end();
                    activeSize += active.size();
                    activeQuery++;
                }
            }
        }

#ifdef DEBUG_PRINT
        std::cout << " actives: " << active.size() << '\n';
#endif

        // now intersect each passing segment with others at same y
        // we intersected only passing/transitional segments paricipating in events.
        // some of them may not be in events
        // we need to collect all of them for each y and intersect n^2

        // find all passing (transitional) segments at each y
        // since for now we have only transitional segments
        // just get one matching segment for current y and traverse up and down
        // along actives while in same y
        {
            static std::vector<ESegment*> transitionalSegments;
            auto processTransitionals = [&]() {
                auto seed = transitionalSegments[0]->activeIt;
                double ySeed = (*seed)->getY(sweepX);
                auto finish = seed;
                while (true) {
                    if (finish == active.end())
                        break;
                    if (!epsEqual((*finish)->getY(sweepX), ySeed)) {
                        break;
                    }
                    finish++;
                }

                auto start = seed;
                while (true) {
                    if (start == active.begin())
                        break;
                    start--;
                    if (!epsEqual((*start)->getY(sweepX), ySeed)) {
                        start++;
                        break;
                    }
                }

                // now do n^2
                for (auto it0 = start; it0 != finish; it0++) {
                    auto it1 = it0;
                    it1++;
                    for (; it1 != finish; it1++)
                        intersectActives(it0, it1);
                }

                transitionalSegments.clear();
            };
            double yCurr = std::numeric_limits<double>::quiet_NaN();
            for (auto ex : eventsAtCurrentX) {
                if (ex.event->getType() != Event::eTYPE::CROSS)
                    continue;
                if (transitionalSegments.size() == 0 && yCurr != yCurr) {
                    yCurr = ex.y;
                    transitionalSegments.push_back(ex.event->seg0);
                }
                if (yCurr != ex.y) {
                    processTransitionals();
                    yCurr = ex.y;
                    transitionalSegments.push_back(ex.event->seg0);
                }
            }
            if (transitionalSegments.size())
                processTransitionals();
        }

        {
            // insert and intersect
            for (auto it = eventsAtCurrentX.begin(); it != eventsAtCurrentX.end(); it++) {
                if (it->event->getType() == Event::eTYPE::BEGIN) {

#ifdef DEBUG_PRINT
                    DUMPACTIVES
                    std::cout << "inserting " << ID(it->event->seg0) << '\n';
#endif
                    size_t size = active.size();
                    auto newActiveIt = active.insert(it->event->seg0).first;
                    it->event->seg0->activeIt = newActiveIt;
#ifdef DEBUG_PRINT
                    if (active.size() != size + 1)
                        std::cout << "ERROR!\n";

                    DUMPACTIVES
#endif
                    // intersectNeighborsWithMe(newActiveIt);
                    // we need not just to intesect with neighbors but to advance those neighbrs
                    // until we leave current y we need to advance while y is equal plus one more to
                    // get intersections with closest non-degenerated neighbors

                    double neighborY = it->y;
                    auto neighborIt = newActiveIt;
                    while (epsEqual(it->y, neighborY)) {
                        neighborIt++;
                        if (neighborIt != active.end()) {
                            neighborY = (*neighborIt)->getY(sweepX);
                            intersectActives(newActiveIt, neighborIt);
                        } else
                            break;
                    }

                    neighborIt = newActiveIt;
                    neighborY = it->y;
                    while (epsEqual(it->y, neighborY)) {
                        if (neighborIt != active.begin()) {
                            neighborIt--;
                            neighborY = (*neighborIt)->getY(sweepX);
                            intersectActives(newActiveIt, neighborIt);
                        } else
                            break;
                    }
                }
            }
        }

        // now after insertion it is time to intersect with new ones and those who passing this
        // sweepX
        intersectActivesWithVertical();

#ifdef DEBUG_PRINT
        std::cout << " actives: " << active.size() << '\n';
#endif

        // intersect crossing segments with new observed neighbors
        for (auto ex : eventsAtCurrentX) {
#ifdef DEBUG_PRINT
            DUMPACTIVES
#endif
            // intersect
            if (ex.event->getType() != Event::eTYPE::CROSS)
                continue;
#ifdef DEBUG_PRINT
            std::cout << "intersect " << ID(ex.event->seg0) << ' ' << ID(ex.event->seg1) << '\n';
#endif
            // OPTIMIZATION: skip intersection of seg0 and seg1, they ought to be ordered so...
            if (ex.event->seg0->activeIt != active.end() &&
                ex.event->seg1->activeIt != active.end()) {
                intersectNeighborsWithMe(ex.event->seg0->activeIt);
                intersectNeighborsWithMe(ex.event->seg1->activeIt);
            }

            // now intersect with

#ifdef DEBUG_PRINT
            DUMPACTIVES
#endif
        }

#ifdef DEBUG_PRINT
        std::cout << " actives: " << active.size() << '\n';
#endif

        // proceed to next event (possibly newly added)
        it++;
        // clear all events that we have processed and add current event to activeEventsAtCurrentX
        eventsAtCurrentX.clear();
        verticalSegmentsAtCurrentX.clear();
        sweepX = it->val;
        iter++;
#ifdef DEBUG_PRINT
        std::cout << " actives: " << active.size() << '\n';
#endif
    }

#ifdef DEBUG_PRINT
    std::cout << "intersection time is " << tim.elapsed() << '\n';
#endif
    tim.reset();

    res.removeDuplicates();
#ifdef DEBUG_PRINT
    std::cout << "remove dumplicates time is " << tim.elapsed() << '\n';
#endif
    tim.reset();

#ifdef DEBUG_PRINT
    std::cout << "FAILURES: " << big(failures) << '\n';
    std::cout << "UPDATES: " << big(updatesCounter) << '\n';
    std::cout << "eventsCompareCounter: " << big(eventsCompareCounter) << '\n';
    std::cout << "OCROSS: " << big(orderedCrossCounter) << '\n';
    std::cout << "OCROSS FINAL: " << big(orderedCrossCounterFinal) << '\n';
    std::cout << "ACTUAL CROSS POINTS: " << big(res.size()) << '\n';
    std::cout << "inserts: " << big(insertionsCounter) << '\n';
    std::cout << "erase: " << big(eraseCounter) << '\n';
    std::cout << "updateOnInsertCounter: " << big(updateOnInsertCounter) << '\n';
    std::cout << "updateOnEraseCounter: " << big(updateOnEraseCounter) << '\n';
    std::cout << "active query: " << big(activeQuery) << '\n';
    std::cout << "active mean size: " << (double)activeSize / (double)activeQuery << '\n';
    std::cout << "active max size: " << big(activeSizeMax) << '\n';
    std::cout << "expect: "
              << std::log((double)activeSize / (double)activeQuery) / std::log(2.) * 3 * activeQuery
              << '\n';
    std::cout << "allocatedItemsCounter: " << big(allocatedItemsCounter) << '\n';
    std::cout << "actives in the end: " << active.size() << '\n';
    std::cout << "vertical actives max size: " << big(verticalActivesMaxSize) << '\n';
    std::cout << "vetrical max size: " << big(verticalAtXmaxSize) << '\n';
    // std::cout << "ACTUAL CROSS POINTS REF: " << evs.size() << '\n';
#endif

    return res;
}

} // namespace internal
} // namespace gkernel

#endif // GKERNEL_INTERNAL_ALGO_SWPL_HPP
