//
// Created by etoga on 5/12/23.
//

#ifndef DYNAMICCONVEXHULL_RANKHULLTREE_H
#define DYNAMICCONVEXHULL_RANKHULLTREE_H

#include <array>
#include <vector>
#include <numeric>
#include "AvlTree.h"
#define isLeaf this->isLeaf

template<typename T>
struct Quotient{
using S = typename std::make_signed<T>::type;

    S n;
    S d;

    Quotient() = default;

    explicit Quotient(T x): n(x),d(1){};

    Quotient(T n, T d): n(n), d(d){reduce();};

    inline
    void reduce(){
        S gcd = std::gcd(n,d);
        n/=gcd;
        d/=gcd;
    }

    Quotient& operator+=(const Quotient& rhs){
        n = n*rhs.d+rhs.n*d;
        d *= rhs.d;
        reduce();
        return *this;
    }

    Quotient& operator-=(const Quotient& rhs){
        n = n*rhs.d-rhs.n*d;
        d *= rhs.d;
        reduce();
        return *this;
    }

    Quotient& operator*=(const Quotient& rhs){
        n *= rhs.n;
        d *= rhs.d;
        reduce();
        return *this;
    }

    Quotient& operator/=(const Quotient& rhs){
        n *= rhs.d;
        d *= rhs.n;
        reduce();
        return *this;
    }

    friend Quotient operator+(Quotient lhs, const Quotient& rhs){
        lhs += rhs;
        return lhs;
    }

    friend Quotient operator-(Quotient lhs, const Quotient& rhs){
        lhs -= rhs;
        return lhs;
    }

    friend Quotient operator*(Quotient lhs, const Quotient& rhs){
        lhs *= rhs;
        return lhs;
    }

    friend Quotient operator/(Quotient lhs, const Quotient& rhs){
        lhs /= rhs;
        return lhs;
    }

    friend bool operator==(const Quotient& lhs, const Quotient& rhs){ return lhs.n * rhs.d == rhs.n * lhs.d; }
    friend bool operator<(const Quotient& lhs, const Quotient& rhs){ return lhs.n * rhs.d < rhs.n * lhs.d; }
    friend bool operator>(const Quotient& lhs, const Quotient& rhs){ return rhs < lhs; }
    friend bool operator<=(const Quotient& lhs, const Quotient& rhs){ return !(lhs > rhs); }
    friend bool operator>=(const Quotient& lhs, const Quotient& rhs){ return !(lhs < rhs); }

};

template<typename T>
struct Bridge{
    T l,r; // Left and Right values
    uint ldiff,rdiff; // Difference in rank to median

    explicit Bridge(T x): l(x),r(x),ldiff(0),rdiff(0){};

    Bridge(T l, T r, uint ldiff, uint rdiff): l(l), r(r), ldiff(ldiff), rdiff(rdiff){};

    friend bool operator<(const Bridge& lhs, const Bridge& rhs){ return lhs.l < rhs.l; }
    friend bool operator==(const Bridge& lhs, const Bridge& rhs){ return lhs.l == rhs.l && lhs.r == rhs.r; }
};

template<typename T>
using Bridges = std::array<Bridge<T>,2>;

template<typename T>
struct Point{
    Quotient<T> x,y;

    Point() = default;

    Point(T x, T y): x(Quotient(x)),y(Quotient(y)){};

    Point(Quotient<T> x, Quotient<T> y): x(x),y(y){};
};

template<typename T>
struct Segment{
    Point<T> l,r;
    Quotient<T> slope;

    Segment()= default;

    Segment(const Bridge<T>& b, T offset) : Segment(b,offset,0){}

    // TODO: y_offset might be overflown unsigned value
    Segment(const Bridge<T>& b, T x_offset, T y_offset) : Segment(
                Point(x_offset-b.ldiff, b.l + y_offset),
                Point(x_offset+b.rdiff, b.r + y_offset)
                ){}

    Segment(const Point<T> l, const Point<T> r): l(l),r(r){
        if(l.x == r.x) slope = Quotient<T>(0);
        else slope =  (r.y-l.y)/(r.x-l.x);
    };

    // TODO: Does not work for q left of l if T unsigned
    inline
    Point<T> eval(Quotient<T> q) const {
        return Point<T>(q, l.y + slope * (q-l.x));
    }

    inline
    Point<T> midpoint() const {
        return eval(l.x+(r.x-l.x) * Quotient<T>(1,2));
    }
};

template<typename T, int epsilon>
class RHTree : AVLTree<Bridges<T>>{
using AVL = AVLTree<Bridges<T>>;
using Node = typename AVL::Node;
using Bridge = Bridge<T>;
using Segment = Segment<T>;
using Point = Point<T>;
using Quotient = Quotient<T>;

/*using Midpoint = typename Traits::Construct_midpoint_2;
using Compare_slope = typename Traits::Compare_slope_2;
using Compare_at_x = typename Traits::Compare_y_at_x_2;
using Left_turn = typename Traits::Left_turn_2;

const Midpoint midpoint = Midpoint();
const Compare_slope compare_slope = Compare_slope();
const Compare_at_x compare_at_x = Compare_at_x();
const Left_turn left_turn = Left_turn();
*/

protected:
/*
    bool slope_comp(const Bridge& l, const Bridge& r, const bool lower){
        CGAL::Comparison_result res = compare_slope(l,r);
        if(res == CGAL::EQUAL) return true;
        return (res == CGAL::SMALLER) != lower;
    }

    // TODO: Naming probably shouldnt overlap like this
    bool slope_comp(const Line& l, const Line& r, bool& parallel){
        CGAL::Comparison_result res = compare_slope(l,r);
        parallel = res == CGAL::EQUAL;
        return res == CGAL::SMALLER;
    }

    bool m_comp(const Bridge& l, const Bridge& r, const Point& m, const bool lower){
        if(l.is_vertical()) return lower;
        if(r.is_vertical()) return !lower;
        CGAL::Comparison_result res = compare_at_x(m,l.supporting_line(),r.supporting_line());
        if(res == CGAL::EQUAL) return true;
        return (res == CGAL::SMALLER) == lower;
    }

    bool cover_comp(const Point& p, const Bridge& b, const bool lower){
        CGAL::Comparison_result res = compare_at_x(p,b.supporting_line());
        if(res == CGAL::EQUAL) return true;
        return (res == CGAL::SMALLER) == lower;
    }
    */
    // This is doable without signed positions by proper cyclic shifts of input
    bool left_turn(const Point& a, const Point& b, const Point& c){
        return (a.x - c.x)*(b.y-c.y) > (b.x - c.x)*(a.y - c.y);
    }

    inline
    bool direction_comp(const Segment& l, const Segment& r, bool& parallel){
        parallel = (l.slope == r.slope);
        return l.slope < r.slope;
    }

    inline
    bool slope_comp(const Segment& l, const Segment& r, const bool lower){
        if(l.slope == r.slope) return true;
        else return (l.slope < r.slope) != lower;
    }

    inline
    bool m_comp(const Segment& l, const Segment& r, const Quotient& q, const bool lower){
        if(l.slope == Quotient(0)) return lower;
        if(r.slope == Quotient(0)) return !lower;
        Point lm = l.eval(q);
        //Quotient rm = lm.y+r.slope()*(r.l.x-q);
        Point rm = r.eval(q);
        if(lm.y== rm.y) return true;
        return (lm.y < rm.y) == lower;
    }

    Bridge findBridge(Node* v, const bool lower){
        Node* x = v->left;
        Node* y = v->right;
        Segment e_l, e_r, lr;
        Point l_m,r_m;
        bool undecided,dirty_l = true, dirty_r = true;
        uint v_rank = 1 + AVL::size(x), l_rank = 1 + AVL::size(x->left), r_rank = 1 + AVL::size(x) + AVL::size(y->left);

        Quotient m = Quotient(x->size) + Quotient(1,2);

        while (!(isLeaf(x) && isLeaf(y))) {
            undecided = true;
            if(dirty_l){
                e_l = Segment(x->val[lower],l_rank);
                l_m = e_l.midpoint();
                dirty_l = false;
            }
            if(dirty_r){
                e_r = Segment(y->val[lower],r_rank);
                r_m = e_r.midpoint();
                dirty_r = false;
            }
            lr = Segment(l_m,r_m);
            if (!isLeaf(x) && slope_comp(e_l,lr,lower)){
                x = stepLeft(x,lower,l_rank); undecided = false; dirty_l = true;
            }
            if (!isLeaf(y) && slope_comp(lr,e_r,lower)) {
                y = stepRight(y,lower,r_rank); undecided = false; dirty_r = true;
            }
            if (undecided) {
                if (!isLeaf(x) && m_comp(e_l,e_r,m,lower) || isLeaf(y)) {
                    x = stepRight(x,lower,l_rank); dirty_l = true;
                } else {
                    y = stepLeft(y,lower,r_rank); dirty_r = true;
                }
            }
        }
        return Bridge(x->val[lower].l,y->val[lower].r,v_rank - l_rank, r_rank - v_rank);
    }

    void onUpdate(Node* x){
        if(isLeaf(x)) return;
        x->val[0] = findBridge(x,0);
        x->val[1] = findBridge(x,1);
    }

    inline
    Node* stepLeft(Node* v, const bool lower){
        auto x = v->val[lower].l;
        v = v->left;
        while(v && v->val[lower].r > x) v = v->left;
        return v;
    }
    inline
    Node* stepLeft(Node* v, const bool lower, uint& rank){
        auto x = rank - v->val[lower].ldiff;
        v = v->left;
        while(v){
            rank -= AVL::size(v->right) + isLeaf(v);
            if(rank + v->val[lower].rdiff <= x) break;
            v = v->left;
        }
        return v;
    }

    inline
    Node* stepRight(Node* v, const bool lower){
        auto x = v->val[lower].r;
        v = v->right;
        while(v && v->val[lower].l < x) v = v->right;
        return v;
    }
    inline
    Node* stepRight(Node* v, const bool lower, uint& rank){
        auto x = rank + v->val[lower].rdiff;
        v = v->right;
        while(v){
            rank += AVL::size(v->left);
            if(rank - v->val[lower].ldiff >= x) break;
            v = v->right;
        }
        return v;
    }

    /*
    // TODO: Inteded not to use stepRight/Left?
    Node* find(const Point key, const bool left, const bool lower){
        auto current = AVLTree<Bridges<T>>::root;
        while(current && !isLeaf(current)){
            if(current->val[lower][!left] == key) return current;
            else if (current->val[lower].min().x() < key.x()) current = current->right;
            else current = current->left;
        }
        return nullptr;
    }

    Node* hullSuccessor(const Point key, const bool lower){
        auto current = AVLTree<Bridges<T>>::root;
        Point min;
        while(current){
            min = current->val[lower].min();
            if(min == key) break;
            else if (min < key) current = stepRight(current,lower);
            else current = stepLeft(current,lower);
        }
        return current;
    }

    Node* hullSuccessor(const Node* p, const bool lower){
        return hullSuccessor(p->val[lower].max(), lower);
    }
    Node* hullPredecessor(const Point key, const bool lower){
        auto current = AVLTree<Bridges<T>>::root;
        Point max;
        while(current){
            max = current->val[lower].max();
            if(max == key) break;
            else if (max < key) current = stepRight(current,lower);
            else current = stepLeft(current,lower);
        }
        return current;
    }

    Node* hullPredecessor(const Node* p, const bool lower){
        return hullPredecessor(p->val[lower].min(), lower);
    }
     */
    /*
    Node* findLower(Point<T> key, bool left){
        auto current = AVLTree<Bridges<T>>::root;
        while(current && !isLeaf(current)){
            if((left && current->val.lower.a == key) || (!left && current->val.lower.b == key)) return current;
            else if (current->val.lower.a.x < key.x) current = current->right;
            else current = current->left;
        }
        return nullptr;
    }
     */

    /*
    bool covers(Point p, const bool lower){
        auto current = AVLTree<Bridges<T>>::root;
        while(current){
            if(current->val[lower].min() <= p){
                if(p <= current->val[lower].max()){
                    return cover_comp(p,current->val[lower],lower);
                } else current = stepRight(current,lower);
            } else current = stepLeft(current,lower);
        }
        return false;
    }
    */
    /*
    bool coversLower(Point<T> p){
        auto current = AVLTree<Bridges<T>>::root;
        while(current){
            if(current->val.lower.a.x <= p.x){
                if(p.x <= current->val.lower.b.x){
                    // Check
                    auto l = Line(current->val.lower.a,current->val.lower.b);
                    return p.y >= l.eval(p.x);
                } else current = stepRightLower(current);
            } else current = stepLeftLower(current);
        }
        return false;
    }
     */

    Node* hullSuccessor(const T key, const bool lower){
        auto current = AVL::root;
        T min;
        while(current){
            min = current->val[lower].l;
            if(min == key) break;
            else if (min < key) current = stepRight(current,lower);
            else current = stepLeft(current,lower);
        }
        return current;
    }


    std::vector<T> hullPoints(const bool lower){
        std::vector<T> res;
        Node* e = AVL::root;
        if(!e) return res;
        res.push_back(e->min[lower].l);
        while(!isLeaf(e)){
            e = hullSuccessor(res.back(),lower);
            res.push_back(e->val[lower].r);
        }
        res.pop_back();
        if(!lower) std::reverse(res.begin(), res.end());
        return res;
    }

    /*
    // Return the dual of the input
    inline
    Line dual(const Point& p, const int& offset){
        return Line(p.hx(),-p.hw(),-p.hy()-offset);
    }

    inline
    Point dual(const Line& l, const int& offset){
        return Point(l.a(),-l.c()+offset*l.b(),-l.b());
    }

    inline
    Point median(Node* p, const bool lower){
        if(p->right) p = p->right;
        return p->min[lower].min();
    }
    */

public:
    void insert(T x){
        AVLTree<Bridges<T>>::insert({Bridge(x),Bridge(x)});
    }

    void remove(T x){
        AVLTree<Bridges<T>>::remove({Bridge(x),Bridge(x)});
    }

    /*
    bool covers(Point p){
        return covers(p,0) && covers(p,1);
    }
    */
    std::vector<T> upperHullPoints(){
        return hullPoints(false);
    }

    std::vector<T> lowerHullPoints(){
        return hullPoints(true);
    }

    // Computes a line that intersects all segments
    // Returns true if successful, storing such a line in the parameter
    // Returns false if unsuccessful, will not modify line in this case
    bool findLine(){
        Node* x = AVL::root; // Upper bridge - lower envelope in dual
        Node* y = AVL::root; // Lower bridge - upper envelope in dual
        uint u_rank = 1 + AVL::size(x);
        uint l_rank = u_rank;
        Segment u,l;
        bool ul_in_l,ur_in_l,ll_in_u,lr_in_u; // Upper Left below L, ... , Lower Left above U, ...
        bool seekLeft, parallel;
        while(x && y && !isLeaf(x) && !isLeaf(y)){ // TODO: Only update moved segments
            // Get predecessors
            //x_succ = hullSuccessor(x,0);
            //y_pred = hullPredecessor(y,1);

            // Get the line segment on lower envelope in dual
            u = Segment(x->val[0],u_rank,-epsilon);
            l = Segment(y->val[1],l_rank,epsilon);

            // Determine direction of (possible) intersection
            seekLeft = direction_comp(u,l,parallel);

            // Unbounded rays towards right
            /*
            if(isLeaf(x) && isLeaf(y)){
                if(seekLeft) return false;
                result = Line(x->val[0].max(),y->val[1].min());
                return true;
            }*/

            // Determine regions
            ul_in_l = left_turn(l.l,l.r,u.l);
            ur_in_l = left_turn(l.l,l.r,u.r);
            ll_in_u = !left_turn(u.l,u.r,l.l);
            lr_in_u = !left_turn(u.l,u.r,l.r);

            if(parallel){
                if(ur_in_l) return false;
                //result = x->val[0];
                return true;
            } else if(ul_in_l != ur_in_l && ll_in_u != lr_in_u) { // Line segments intersect
                return false;
            }

            // TODO: Check that directions are true, no longer in dual
            if(seekLeft){ // If slope of L strictly less than U then intersection region is left
                if(!ul_in_l) x = stepLeft(x,0,u_rank); // Case 1
                if(!ll_in_u) y = stepLeft(y,1,l_rank); // Case 2
                if(ul_in_l && ll_in_u) { // Case 3
                    if(u.r.x > l.r.x) x = stepLeft(x,0,u_rank);
                    else y = stepLeft(y,1,l_rank);
                }
            } else {
                if(!ur_in_l) x = stepRight(x,0,u_rank);
                if(!lr_in_u) y = stepRight(y,1,l_rank);
                if(ur_in_l && lr_in_u){
                    if(u.l.x < l.l.x) x = stepRight(x,0,u_rank);
                    else y = stepRight(y,1,l_rank);
                }
            }
        }
        return true;
    }
};
#endif //DYNAMICCONVEXHULL_RANKHULLTREE_H
