cmake_minimum_required(VERSION 3.16)
project(WithEdgeDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(
    synthesis CONFIG REQUIRED
    PATHS "${CMAKE_SOURCE_DIR}/lib/Release/cmake/synthesis"
    NO_DEFAULT_PATH
)

find_package(glog QUIET)
if(NOT TARGET glog::glog)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLOG REQUIRED libglog)
endif()

add_executable(with_edge_demo src/main.cpp)

target_link_libraries(with_edge_demo PRIVATE synthesis::gkernel)

if(TARGET glog::glog)
    target_link_libraries(with_edge_demo PRIVATE glog::glog)
else()
    target_include_directories(with_edge_demo PRIVATE ${GLOG_INCLUDE_DIRS})
    target_link_directories(with_edge_demo PRIVATE ${GLOG_LIBRARY_DIRS})
    target_link_libraries(with_edge_demo PRIVATE ${GLOG_LIBRARIES})
endif()

set_target_properties(with_edge_demo PROPERTIES
    BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib/Release"
)
