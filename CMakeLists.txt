cmake_minimum_required(VERSION 3.24)
project(DynamicLearnedIndex)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3")


add_executable(LearnedIndex
        convex_hull/AvlTree.h
        convex_hull/RankHullTree.h
        learned_index/LearnedIndex.h
        tester.cpp
)

