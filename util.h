#ifndef DYNAMICCONVEXHULL_UTIL_H
#define DYNAMICCONVEXHULL_UTIL_H

template<class T>
struct Point{
    T x,y;

    Point(T x, T y):x(x),y(y) {};

    Point() = default;

    constexpr bool operator==(const Point& p){
        return (x == p.x) && (y == p.y);
    }

    constexpr bool operator!=(const Point& p){
        return !(*this == p);
    }

    constexpr bool operator<(const Point& p){
        return (x < p.x) || (x == p.x && y < p.y);
    }

    constexpr bool operator<=(const Point& p){
        return !(p < *this);
    }
};

template<class T>
struct Bridge{
    Point<T> a,b;

    Bridge(Point<T> a, Point<T> b): a(a),b(b) {};

    constexpr bool operator==(const Bridge& p){
        return (a == p.a) && (b == p.b);
    }

    constexpr bool operator!=(const Bridge& p){
        return !(*this == p);
    }

    constexpr bool operator<(const Bridge& p){
        return (a < p.a);
    }

    constexpr bool operator<=(const Bridge& p){
        return !(p < *this);
    }
};


template<class T>
struct Bridges{
    Bridge<T> upper,lower;

    Bridges(T x, T y):upper({x,y},{x,y}),lower({x,y},{x,y}){};

    constexpr bool operator==(const Bridges& b){
        return upper.a == b.upper.a;
    }

    constexpr bool operator!=(const Bridges& b){
        return !(*this == b);
    }

    constexpr bool operator<(const Bridges& b){
        return upper.a < b.upper.a;
    }

    constexpr bool operator<=(const Bridges& b){
        return !(b < *this);
    }
};



#endif //DYNAMICCONVEXHULL_UTIL_H
