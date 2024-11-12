#ifndef LEARNEDINDEX_H
#define LEARNEDINDEX_H

#include "LineTree.h"
#include "../page_bearer/pbs_map_and_vec.h"

/*
template<class NumType>
struct less{
    bool operator()(const typename Traits::Segment_2& lhs, const NumType& rhs) const {
        return lhs.max().y() < rhs;
    }
};*/



template<typename NumType>
struct LearnedIndex {
    uint epsilon = 256;

    //AVLTree<Segment,less<Traits,NumType>> lines;
    LineTree<NumType,Quotient<NumType>> lineTree = LineTree<NumType,Quotient<NumType>>(epsilon);
    MapAndVecPBS pbs = MapAndVecPBS(epsilon);
    //Line res;

    bool insert(NumType val){
        // Find correct line
        lineTree.insert(val);
        std::cout << lineTree.root->size << std::endl;
        // Insert into hull spanning line
        // Check if line exists
        // If not - insert into first half and check
        // If not - insert into second half and check
        // If not - degenerate 1 point line

        // Find page-bearer by backwards traversal

        // If new element is a page-bearer, split existing page
        return true;
    }
};
#endif //LEARNEDINDEX_H
