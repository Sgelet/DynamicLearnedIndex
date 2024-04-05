#ifndef LEARNEDINDEX_H
#define LEARNEDINDEX_H

#include "../convex_hull/AvlTree.h"
#include "../convex_hull/CHTree.h"
#include "../page_bearer/pbs_map_and_vec.h"

template<class Traits, class NumType>
struct less{
    bool operator()(const typename Traits::Segment_2& lhs, const NumType& rhs) const {
        return lhs.max().y() < rhs;
    }
};

template<class Traits, class NumType, int epsilon>
struct LearnedIndex {
    using Segment = typename Traits::Segment_2;
    using Line = typename Traits::Line_2;

    AVLTree<Segment,less<Traits,NumType>> lines;
    CHTree<Traits,epsilon> hull;
    MapAndVecPBS pbs = MapAndVecPBS(epsilon);
    Line res;

    bool insert(NumType val){
        // Find correct line
        hull.insert({val,val});
        return hull.findLine(res);
        // Insert into hull spanning line
        // Check if line exists
        // If not - insert into first half and check
        // If not - insert into second half and check
        // If not - degenerate 1 point line

        // Find page-bearer by backwards traversal

        // If new element is a page-bearer, split existing page
    }
};
#endif //LEARNEDINDEX_H
