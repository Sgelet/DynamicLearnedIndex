cmake_minimum_required(VERSION 3.24)
project(DynamicConvexHull)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O0")

find_package(CGAL)

add_executable(DynamicConvexHull convex_hull/AvlTree.h convex_hull/CHTree.h convex_hull/smalltest.cpp util.h)
add_executable(PageBearer page_bearer/main.cpp)
add_executable(LearnedIndex
        page_bearer/pbs.h
        learned_index/LearnedIndex.h
        learned_index/tester.cpp)

target_link_libraries(DynamicConvexHull CGAL::CGAL)
target_link_libraries(LearnedIndex CGAL::CGAL)