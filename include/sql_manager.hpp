#ifndef CORE_SQL_MANAGER_H
#define CORE_SQL_MANAGER_H

#include <map>
#include <string>

#include "base.hpp"

class sqlite3;

namespace core {

class SqlManager {
public:
    static void init(std::string _path);

    static bool insert(const gkernel::Layer* layers, std::string comment);

    static void close();

    static std::string getDbPath() {
        return dbPath;
    }

    static size_t getInsertRowCount() {
        return insertRowCount;
    }

    static bool isNeedWriteResultDB;

private:
    static sqlite3* db;

    static int checkId;

    static int elementId;

    static std::string path;

    static std::string dbPath;

    static std::map<gkernel::Point, size_t> points;

    static size_t insertRowCount;

    static void execCommand(std::string command);

    static size_t getPointId(const gkernel::Point& pt, std::stringstream& insertStream);

    static bool init();
};

} // namespace core

#endif // CORE_SQL_MANAGER_H