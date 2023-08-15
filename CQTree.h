//
// Created by etoga on 5/12/23.
//

#ifndef DYNAMICCONVEXHULL_CQTREE_H
#define DYNAMICCONVEXHULL_CQTREE_H

#include <vector>
#include "AvlTree.h"
#include "util.h"
#define isLeaf this->isLeaf

template<class Traits>
struct comp_xy{
    static constexpr auto comp = typename Traits::Compare_xy_2();
    bool operator()(const typename Traits::Segment_2& lhs, const typename Traits::Segment_2& rhs) const {
        return comp(lhs[0],rhs[0]) == CGAL::SMALLER;
    }
};

template<class Traits>
using CQueue = AVLTree<typename Traits::Segment_2,comp_xy<Traits>>;

template<class Traits>
struct BCQ{
    using Bridge = typename Traits::Segment_2;
    Bridges<Traits> bridges;
    std::array<CQueue<Traits>,2> hulls;

    BCQ(Bridge x, Bridge y):bridges(x,y){};

    BCQ(BCQ& b):bridges(b.bridges){};

    constexpr bool operator==(const BCQ& b) const{
        return bridges == b.bridges;
    }

    constexpr bool operator!=(const BCQ& b) const{
        return !(*this == b);
    }

    constexpr bool operator<(const BCQ& b) const{
        return bridges < b.bridges;
    }

    constexpr bool operator<=(const BCQ& b) const{
        return !(b < *this);
    }
};

template<class Traits>
class CQTree : AVLTree<BCQ<Traits>>{
using Node = typename AVLTree<BCQ<Traits>>::Node;
using HNode = typename CQueue<Traits>::Node;
using Bridge = typename Traits::Segment_2;
using Point = typename Traits::Point_2;
using Bridges = Bridges<Traits>;
using Midpoint = typename Traits::Construct_midpoint_2;
using Compare_slope = typename Traits::Compare_slope_2;
using Compare_at_x = typename Traits::Compare_y_at_x_2;

const Midpoint midpoint = Midpoint();
const Compare_slope compare_slope = Compare_slope();
const Compare_at_x compare_at_x = Compare_at_x();

protected:
    // TODO: Degeneracy check
    bool slope_comp(const Bridge& l, const Bridge& r, const bool lower){
        CGAL::Comparison_result res = compare_slope(l,r);
        if(res == CGAL::EQUAL) return true;
        return (res == CGAL::SMALLER) != lower;
    }

    bool m_comp(const Bridge& l, const Bridge& r, const Point& m, const bool lower) {
        if(l.is_vertical()) return lower;
        if(r.is_vertical()) return !lower;
        CGAL::Comparison_result res = compare_at_x(m, l.supporting_line(), r.supporting_line());
        if (res == CGAL::EQUAL) return true;
        return (res == CGAL::SMALLER) == lower;
    }

    Bridge findBridge(Node* v, const bool lower){
        HNode* x = v->left->val.hulls[lower].root;
        HNode* y = v->right->val.hulls[lower].root;
        Bridge e_l, e_r, lr;
        bool undecided;
        bool foundl = false;
        bool foundr = false;
        Point l,r;
        Point m = midpoint(v->left->max.bridges[lower].max(),v->right->min.bridges[lower].min());
        if(!x){
            foundl = true;
            l = v->left->val.bridges[lower].min();
        }
        if(!y){
            foundr = true;
            r = v->right->val.bridges[lower].min();
        }
        while (!foundl || !foundr) {
            undecided = true;
            if(!foundl){
                e_l = x->val;
                l = midpoint(x->val);
            }
            if(!foundr) {
                e_r = y->val;
                r = midpoint(y->val);
            }
            lr = Bridge(l,r);
            if (!foundl && slope_comp(e_l,lr,lower)){
                if(x->left) x = x->left;
                else{
                    foundl = true;
                    l = x->val.min();
                }
                undecided = false;
            }
            if (!foundr && slope_comp(lr,e_r,lower)) {
                if(y->right) y = y->right;
                else{
                    foundr = true;
                    r = y->val.max();
                }
                undecided = false;
            }
            if (undecided) {
                //T m = avgX(foundl ? l : x->val.b,foundr ? r: y->val.a);
                if (foundr ||  (!foundl && m_comp(e_l,e_r,m,lower))) {
                    if(x->right) x = x->right;
                    else{
                        foundl = true;
                        l = x->val.max();
                    }
                } else {
                    if(y->left) y = y->left;
                    else{
                        foundr = true;
                        r = y->val.min();
                    }
                }
            }
        }
        return {l,r};
    }

    void onUpdate(Node* x){
        if(isLeaf(x)) return;
        auto ub = findBridge(x,0);
        auto lb = findBridge(x,1);
        x->val.bridges[0] = ub;
        x->val.bridges[1] = lb;
        CQueue<Traits> left, right;
        for(int i=0; i<2; ++i){
            x->left->val.hulls[i].split(x->val.bridges[i],&left);
            x->right->val.hulls[i].split(x->val.bridges[i],&right);
            x->left->val.hulls[i].join(x->val.bridges[i],&right);
            x->val.hulls[i].join(&(x->left->val.hulls[i]));
            x->left->val.hulls[i].join(&left);
        }
    }

    // TODO: Split
    void onVisit(Node* x){
       if(isLeaf(x)) return;
       CQueue<Traits> right;
       for(int i=0;i<2;++i){
           x->val.hulls[i].split(x->val.bridges[i],&right);
           x->val.hulls[i].join(&(x->left->val.hulls[i]));
           x->left->val.hulls[i].join(&(x->val.hulls[i]));
           x->right->val.hulls[i].join(&right);
       }
    }
    /*
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
*/
public:
    void insert(Point p){
        AVLTree<BCQ<Traits>>::insert(BCQ<Traits>(Bridge(p,p),Bridge(p,p)));
    }

    void remove(Point p){
        AVLTree<BCQ<Traits>>::remove(BCQ(Bridge(p,p),Bridge(p,p)));
    }
/*
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
    */
};
#endif //DYNAMICCONVEXHULL_CQTREE_H
