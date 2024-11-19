cmake_minimum_required(VERSION 3.24)
project(DynamicLearnedIndex)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3")

find_package(CGAL)

add_executable(DynamicConvexHull convex_hull/AvlTree.h convex_hull/smalltest.cpp util.h
        convex_hull/RankHullTree.h
        convex_hull/CHTree.h)
add_executable(PageBearer page_bearer/main.cpp)
add_executable(LearnedIndex
        convex_hull/AvlTree.h
        convex_hull/RankHullTree.h
        learned_index/LearnedIndex.h
        learned_index/tester.cpp
        learned_index/Page.h
)

target_link_libraries(DynamicConvexHull CGAL::CGAL)
#target_link_libraries(LearnedIndex CGAL::CGAL)
