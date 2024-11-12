#ifndef DYNAMICCONVEXHULL_UTIL_H
#define DYNAMICCONVEXHULL_UTIL_H

#include <CGAL/enum.h>

template<class Traits>
struct CBridges{
    using Compare = typename Traits::Compare_xy_2;
    using Bridge = typename Traits::Segment_2;

    static constexpr Compare compare = Compare();

    std::array<Bridge,2> data;

    CBridges(Bridge x, Bridge y):data({x,y}){};

    Bridge& operator[](size_t idx){return data[idx%2];}
    const Bridge& operator[](size_t idx) const {return data[idx%2];}

    bool operator==(const CBridges& b) const{
        return (compare(data[0][0],b[0][0]) == CGAL::EQUAL);
    }

    bool operator!=(const CBridges& b) const{
        return !(*this == b);
    }
    bool operator<(const CBridges& b) const{
        return (compare(data[0][0],b[0][0]) == CGAL::SMALLER);
    }

    bool operator<=(const CBridges& b) const{
        return (compare(data[0][0],b[0][0]) != CGAL::LARGER);
    }
};


#endif //DYNAMICCONVEXHULL_UTIL_H
