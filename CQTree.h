//
// Created by etoga on 5/12/23.
//

#ifndef DYNAMICCONVEXHULL_CQTREE_H
#define DYNAMICCONVEXHULL_CQTREE_H

#include <vector>
#include "AvlTree.h"
#define isLeaf this->isLeaf

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

template<class T>
using CQueue = AVLTree<Bridge<T>>;

template<class T>
struct BCQ{
    Bridges<T> bridges;
    CQueue<T> uh;
    CQueue<T> lh;

    BCQ(T x, T y):bridges(x,y){};

    BCQ(BCQ& b):bridges(b.bridges){};

    constexpr bool operator==(const BCQ& b){
        return bridges == b.bridges;
    }

    constexpr bool operator!=(const BCQ& b){
        return !(*this == b);
    }

    constexpr bool operator<(const BCQ& b){
        return bridges < b.bridges;
    }

    constexpr bool operator<=(const BCQ& b){
        return !(b < *this);
    }
};

template<class T>
class CQTree : AVLTree<BCQ<T>>{
using Node = typename AVLTree<BCQ<T>>::Node;
using HNode = typename CQueue<T>::Node;

protected:
    struct Line{
        double slope,offset;

        double eval(const T& x){ return slope*x+offset; }

        Line() = default;

        Line(const Point<T>& v0, const Point<T>& v1){
            slope = v0.x == v1.x ? 0 : (v1.y - v0.y)/(v1.x - v0.x);
            offset = v0.y - slope * v0.x;
        }
    };

    T avgX(const Point<T>& l, const Point<T>& r){
        return l.x+(r.x - l.x)/2;
    }


    Bridge<T> findUpperBridge(Node* v){
        HNode* x = v->left->val.uh.root;
        HNode* y = v->right->val.uh.root;
        Line e_l, e_r, lr;
        T lx, rx, m;
        Point<T> l,r;
        bool undecided;
        bool foundl = false;
        bool foundr = false;
        m = avgX(v->left->max.bridges.upper.b,v->right->min.bridges.upper.a);
        if(!x){
            foundl = true;
            l = v->left->val.bridges.upper.a;
        }
        if(!y){
            foundr = true;
            r = v->right->val.bridges.upper.a;
        }
        while (!foundl || !foundr) {
            undecided = true;
            if(!foundl){
                e_l = Line(x->val.a,x->val.b);
                lx = avgX(x->val.a,x->val.b);
                l = {lx,e_l.eval(lx)};
            }
            if(!foundr) {
                e_r = Line(y->val.a, y->val.b);
                rx = avgX(y->val.a, y->val.b);
                r = {rx, e_r.eval(rx)};
            }
            lr = Line(l,r);
            if (e_l.slope <= lr.slope && !foundl){
                if(x->left) x = x->left;
                else{
                    foundl = true;
                    l = x->val.a;
                }
                undecided = false;
            }
            if (lr.slope <= e_r.slope && !foundr) {
                if(y->right) y = y->right;
                else{
                    foundr = true;
                    r = y->val.b;
                }
                undecided = false;
            }
            if (undecided) {
                //T m = avgX(foundl ? l : x->val.b,foundr ? r: y->val.a);
                auto a = e_l.eval(m);
                auto b = e_r.eval(m);
                if (!foundl && a >= b || foundr) {
                    if(x->right) x = x->right;
                    else{
                        foundl = true;
                        l = x->val.b;
                    }
                } else {
                    if(y->left) y = y->left;
                    else{
                        foundr = true;
                        r = y->val.a;
                    }
                }
            }
        }
        return {l,r};
    }

    Bridge<T> findLowerBridge(Node* v){
        HNode* x = v->left->val.lh.root;
        HNode* y = v->right->val.lh.root;
        Line e_l, e_r, lr;
        T lx, rx, m;
        Point<T> l,r;
        bool undecided;
        bool foundl = false;
        bool foundr = false;
        m = avgX(v->left->max.bridges.lower.b,v->right->min.bridges.lower.a);
        if(!x){
            foundl = true;
            l = v->left->val.bridges.lower.a;
        }
        if(!y){
            foundr = true;
            r = v->right->val.bridges.lower.a;
        }
        while (!foundl || !foundr) {
            undecided = true;
            if(!foundl){
                e_l = Line(x->val.a,x->val.b);
                lx = avgX(x->val.a,x->val.b);
                l = {lx,e_l.eval(lx)};
            }
            if(!foundr) {
                e_r = Line(y->val.a, y->val.b);
                rx = avgX(y->val.a, y->val.b);
                r = {rx, e_r.eval(rx)};
            }
            lr = Line(l,r);
            if (e_l.slope >= lr.slope && !foundl){
                if(x->left) x = x->left;
                else{
                    foundl = true;
                    l = x->val.a;
                }
                undecided = false;
            }
            if (lr.slope >= e_r.slope && !foundr) {
                if(y->right) y = y->right;
                else{
                    foundr = true;
                    r = y->val.b;
                }
                undecided = false;
            }
            if (undecided) {
                auto a = e_l.eval(m);
                auto b = e_r.eval(m);
                if (!foundl && a <= b || foundr) {
                    if(x->right) x = x->right;
                    else{
                        foundl = true;
                        l = x->val.b;
                    }
                } else {
                    if(y->left) y = y->left;
                    else{
                        foundr = true;
                        r = y->val.a;
                    }
                }
            }
        }
        return {l,r};
    }

    void onUpdate(Node* x){
        if(isLeaf(x)) return;
        auto ub = findUpperBridge(x);
        auto lb = findLowerBridge(x);
        x->val.bridges.upper = ub;
        x->val.bridges.lower = lb;
        CQueue<T> left, right;
        x->left->val.uh.split({ub.a,ub.a},&left);
        x->right->val.uh.split({ub.b,ub.b},&right);
        x->left->val.uh.join(ub,&right);
        x->val.uh.join(&(x->left->val.uh));
        x->left->val.uh.join(&left);
        // Lower
        x->left->val.lh.split({lb.a,lb.a},&left);
        x->right->val.lh.split({lb.b,lb.b},&right);
        x->left->val.lh.join(lb,&right);
        x->val.lh.join(&(x->left->val.lh));
        x->left->val.lh.join(&left);}

    // TODO: Split
    void onVisit(Node* x){
       if(isLeaf(x)) return;
       CQueue<T> right;
       // Upper
       x->val.uh.split(x->val.bridges.upper,&right);
       x->val.uh.join(&(x->left->val.uh));
       x->left->val.uh.join(&(x->val.uh));
       x->right->val.uh.join(&right);
       // Lower
       x->val.lh.split(x->val.bridges.lower,&right);
       x->val.lh.join(&(x->left->val.lh));
       x->left->val.lh.join(&(x->val.lh));
       x->right->val.lh.join(&right);
    }
    void hullPoints(HNode* x, std::vector<std::pair<T,T>>& acc){
        if(!x) return;
        hullPoints(x->left,acc);
        acc.emplace_back(x->val.a.x,x->val.a.y);
        hullPoints(x->right,acc);
    }
    void hullPoints2(HNode* x, std::vector<std::pair<T,T>>& acc){
       if(!x) return;
       hullPoints2(x->right,acc);
       acc.emplace_back(x->val.b.x,x->val.b.y);
       hullPoints2(x->left,acc);
    }

    bool coversUpper(Point<T> p){
        HNode* current = AVLTree<BCQ<T>>::root->val.uh.root;
        while(current){
            if(current->val.a.x <= p.x){
                if(p.x <= current->val.b.x){
                    // Check
                    auto l = Line(current->val.a,current->val.b);
                    return p.y <= l.eval(p.x);
                } else current = current->right;
            } else current = current->left;
        }
        return false;
    }
    bool coversLower(Point<T> p){
        HNode* current = AVLTree<BCQ<T>>::root->val.lh.root;
        while(current){
            if(current->val.a.x <= p.x){
                if(p.x <= current->val.b.x){
                    // Check
                    auto l = Line(current->val.a,current->val.b);
                    return p.y >= l.eval(p.x);
                } else current = current->right;
            } else current = current->left;
        }
        return false;
    }

public:
    void insert(T x, T y){
        AVLTree<BCQ<T>>::insert(BCQ(x,y));
    }

    void remove(T x, T y){
        AVLTree<BCQ<T>>::remove(BCQ(x,y));
    }

    bool covers(T x, T y){
        auto p = Point(x,y);
        return coversUpper(p) && coversLower(p);
    }

    std::vector<std::pair<T,T>> upperHullPoints(){
        std::vector<std::pair<T,T>> res;
        if(AVLTree<BCQ<T>>::root) hullPoints2(AVLTree<BCQ<T>>::root->val.uh.root,res);
        return res;
    }
    std::vector<std::pair<T,T>> lowerHullPoints(){
        std::vector<std::pair<T,T>> res;
        if(AVLTree<BCQ<T>>::root) hullPoints(AVLTree<BCQ<T>>::root->val.lh.root,res);
        return res;
    }
};
#endif //DYNAMICCONVEXHULL_CQTREE_H
