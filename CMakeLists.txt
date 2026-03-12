cmake_minimum_required(VERSION 3.16)
project(WithEdgeDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(with_edge_demo src/main.cpp src/glog_stub.cpp)

target_include_directories(with_edge_demo PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(with_edge_demo PRIVATE ${CMAKE_SOURCE_DIR}/lib/Release/libgkernel.so)

set_target_properties(with_edge_demo PROPERTIES
    BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib/Release"
)
