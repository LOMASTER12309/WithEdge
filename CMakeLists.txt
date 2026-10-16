cmake_minimum_required(VERSION 3.16)
project(WithEdgeDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(
    synthesis CONFIG REQUIRED
    PATHS "${CMAKE_SOURCE_DIR}/lib/Release/cmake/synthesis"
    NO_DEFAULT_PATH
)

find_package(OpenMP REQUIRED)

find_package(glog QUIET)
if(NOT TARGET glog::glog)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLOG REQUIRED libglog)
endif()

# Функция для быстрого добавления исполняемых файлов
# Usage: add_app(<name> [src/<name>.cpp])
# Если источник не указан, используется src/<name>.cpp
function(add_app name)
    set(args ${ARGN})
    if(args STREQUAL "")
        set(source "src/${name}.cpp")
    else()
        set(source ${args})
    endif()

    add_executable(${name} ${source})

    target_link_libraries(${name} PRIVATE synthesis::gkernel)
    target_link_libraries(${name} PRIVATE OpenMP::OpenMP_CXX)

    if(TARGET glog::glog)
        target_link_libraries(${name} PRIVATE glog::glog)
    else()
        target_include_directories(${name} PRIVATE ${GLOG_INCLUDE_DIRS})
        target_link_directories(${name} PRIVATE ${GLOG_LIBRARY_DIRS})
        target_link_libraries(${name} PRIVATE ${GLOG_LIBRARIES})
    endif()

    set_target_properties(${name} PROPERTIES
        BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib/Release"
    )
endfunction()

# Основное демо-приложение и ручные тесты
add_app(with_edge_demo src/main.cpp)
add_app(tests_main tests/test_main.cpp)
add_app(test_memory tests/test_memory.cpp)

# Примеры добавления других приложений (раскомментировать при необходимости):
# add_app(main1)           # src/main1.cpp
# add_app(test1)           # src/test1.cpp
# add_app(my_test src/custom_test.cpp)

# Тесты с Catch2 v2 (установлен в /usr/local/include/)
# Установка Catch2 v2:
# 1) cd ~
# 2) git clone --branch v2.13.10 --depth 1 https://github.com/catchorg/Catch2.git
# 3) cd Catch2
# 4) sudo cp -r single_include/catch2 /usr/local/include/
set(CATCH2_INCLUDE_DIR "/usr/local/include")

# Функция для добавления Catch2-теста
# Usage: add_catch_test(<name> [tests/<name>.cpp])
# Если источник не указан, используется tests/<name>.cpp
function(add_catch_test name)
    if(NOT EXISTS "${CATCH2_INCLUDE_DIR}/catch2/catch.hpp")
        message(WARNING "Catch2 v2 not found at ${CATCH2_INCLUDE_DIR}, skipping test '${name}'")
        return()
    endif()

    list(LENGTH ARGN argn_len)
    if(argn_len EQUAL 0)
        set(source "tests/${name}.cpp")
    else()
        set(source ${ARGN})
    endif()

    add_executable(${name} ${source})
    target_include_directories(${name} PRIVATE ${CATCH2_INCLUDE_DIR})
    target_link_libraries(${name} PRIVATE synthesis::gkernel)
    target_link_libraries(${name} PRIVATE OpenMP::OpenMP_CXX)

    if(TARGET glog::glog)
        target_link_libraries(${name} PRIVATE glog::glog)
    else()
        target_include_directories(${name} PRIVATE ${GLOG_INCLUDE_DIRS})
        target_link_directories(${name} PRIVATE ${GLOG_LIBRARY_DIRS})
        target_link_libraries(${name} PRIVATE ${GLOG_LIBRARIES})
    endif()

    set_target_properties(${name} PROPERTIES
        BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib/Release"
    )
endfunction()

# Catch2-тесты (добавляйте новые одной строкой):
add_catch_test(tests tests/tests.cpp)
# add_catch_test(test_something_else)
# add_catch_test(test_new_feature src/custom_path.cpp)

# Примеры добавления других приложений (раскомментировать при необходимости):
# add_app(main1)           # src/main1.cpp
# add_app(test1)           # src/test1.cpp
# add_app(my_test src/custom_test.cpp)
