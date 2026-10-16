#ifndef CORE_LUA_RUNNER_HPP
#define CORE_LUA_RUNNER_HPP

#include <string>

std::pair<std::string, std::string> run(std::string luaPath,
                                        std::string gdsPath,
                                        std::string cellName,
                                        std::string outputPath,
                                        bool isFlat,
                                        bool isNeedGKernelLog,
                                        bool isNeedWriteResultDB);

std::pair<std::string, std::string> runLVS(std::string luaPath,
                                           std::string gdsPath,
                                           std::string cellName,
                                           std::string linkListPath, // Путь к файлу списка связей
                                           std::string headCellName, // Имя головной ячейки
                                           std::string outputPath,
                                           bool isFlat);

#endif // CORE_LUA_RUNNER_HPP