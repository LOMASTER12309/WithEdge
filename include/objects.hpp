#ifndef GDS_READER_OBJECTS_HPP
#define GDS_READER_OBJECTS_HPP

#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

namespace gds_reader {

struct Point {
    double x = 0;
    double y = 0;

    Point() = default;

    Point(double _x, double _y) : x(_x), y(_y) {}
};

struct Contour;
struct Path;
struct Layer;
struct Cell;

struct Contour {
    Layer* layer;

    std::vector<Point> points;

    void normalize();
};

struct Path : public Contour {
    int width = 0;

    Contour convertToContour();
};

struct Text {
    Layer* layer;

    std::string text = "\0";
    int x = 0;
    int y = 0;
};

struct Layer {
    Layer() {}

    explicit Layer(int _id) : id(_id) {}

    Layer(int id, Cell* cell) : Layer(id) {
        this->cell = cell;
    }

    int id;

    Cell* cell{};

    std::vector<Contour> contours;

    std::vector<Text> texts;
};

struct SRef {
    Cell* cell;

    Point pt;

    double angle = 0;

    double scale = 1;

    bool reflection = false;
};

struct ARef : SRef {
    int colCount = 0;

    int rowCount = 0;

    Point ptCornerRow;

    Point ptCornerCol;
};

struct Cell {
    std::string name;

    std::unordered_map<int, Layer> layers;

    std::vector<SRef> sRefs;

    std::vector<ARef> aRefs;

    Cell() {}

    Cell(const Cell& cell)
        : name(cell.name), layers(std::move(cell.layers)), sRefs(std::move(cell.sRefs)),
          aRefs(std::move(cell.aRefs)) {}
};

} // namespace gds_reader

#endif // GDS_READER_OBJECTS_HPP