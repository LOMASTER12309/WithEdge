#ifndef GDS_READER_GDS_MANAGER_HPP
#define GDS_READER_GDS_MANAGER_HPP

#include <map>
#include <string>
#include <sys/stat.h>
#include <vector>

#include "file_reader.hpp"
#include "objects.hpp"

namespace gds_reader {

struct Layer;
struct Cell;
struct SRef;
struct ARef;

class GDSManager {
public:
    static void init(const std::string& path, const std::string& cellName = "");

    static Layer getLayer(int id);

    static int getMaxLayerId();

    static double getDataBaseUnits();

    static double getUserUnits();

    static std::string getFileName() {
        return fileName;
    }

    static std::map<size_t, size_t> getInfo();

    static std::string getRootCellName();

private:
    static std::string path;

    static std::string cellName;

    struct Transform {
        const SRef* sref = nullptr;

        Transform(const SRef* _sref) : sref(_sref) {}

        void operator()(Point& pt);
    };

    static std::vector<Transform> transforms;

    static Cell* root;

    static int maxLayerId;

    static double dataBaseUnits;

    static double userUnits;

    static std::string fileName;

    static std::map<size_t, size_t> info;

    static int getLayer(const fileData& pair, Cell& cell);

    static void getLayer(Cell* cell, int id, Layer& layer);

    static bool read();
};

} // namespace gds_reader

#endif // GDS_READER_GDS_MANAGER_HPP