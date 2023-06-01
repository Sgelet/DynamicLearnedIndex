cmake_minimum_required(VERSION 3.24)
project(DynamicConvexHull)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O2")

find_package(CGAL)

add_executable(DynamicConvexHull AvlTree.h CHTree.h CQTree.h main.cpp)

target_link_libraries(DynamicConvexHull CGAL::CGAL)