cmake_minimum_required(VERSION 3.16)
project(WithEdgeDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(with_edge_demo src/main.cpp)

target_include_directories(with_edge_demo PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(with_edge_demo PRIVATE ${CMAKE_SOURCE_DIR}/lib/Release/libgkernel.so)

find_package(glog CONFIG QUIET)
if(TARGET glog::glog)
    target_link_libraries(with_edge_demo PRIVATE glog::glog)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLOG REQUIRED libglog)
    target_include_directories(with_edge_demo PRIVATE ${GLOG_INCLUDE_DIRS})
    target_link_directories(with_edge_demo PRIVATE ${GLOG_LIBRARY_DIRS})
    target_link_libraries(with_edge_demo PRIVATE ${GLOG_LIBRARIES})
endif()

set_target_properties(with_edge_demo PROPERTIES
    BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib/Release"
)
