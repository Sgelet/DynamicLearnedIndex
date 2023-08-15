//
// Created by etoga on 5/12/23.
//

#ifndef DYNAMICCONVEXHULL_CHTREE_H
#define DYNAMICCONVEXHULL_CHTREE_H

#include <vector>
#include <CGAL/enum.h>
#include "AvlTree.h"
#include "util.h"
#define isLeaf this->isLeaf


template<class Traits>
class CHTree : AVLTree<Bridges<Traits>>{
using Bridge = typename Traits::Segment_2;
using Point = typename Traits::Point_2;
using Bridges = Bridges<Traits>;
using Node = typename AVLTree<Bridges>::Node;
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

    Bridge findBridge(Node* v, const bool lower){
        Node* x = v->left;
        Node* y = v->right;
        Bridge e_l, e_r, lr;
        bool undecided;

        Point m = midpoint(x->max[lower].max(),y->min[lower].min());

        while (!(isLeaf(x) && isLeaf(y))) {
            undecided = true;
            e_l = x->val[lower];
            e_r = y->val[lower];
            lr = Bridge(midpoint(e_l),midpoint(e_r));
            if (!isLeaf(x) && slope_comp(e_l,lr,lower)){
                x = stepLeft(x,lower); undecided = false;
            }
            if (!isLeaf(y) && slope_comp(lr,e_r,lower)) {
                y = stepRight(y,lower); undecided = false;
            }
            if (undecided) {
                if (!isLeaf(x) && m_comp(e_l,e_r,m,lower) || isLeaf(y)) {
                    x = x->right;
                } else {
                    y = y->left;
                }
            }
        }
        return Bridge(x->val[lower].min(),y->val[lower].max());
    }

    void onUpdate(Node* x){
        if(isLeaf(x)) return;
        x->val[0] = findBridge(x,0);
        x->val[1] = findBridge(x,1);
    }

    inline
    Node* stepLeft(Node* v, const bool lower){
        auto x = v->val[lower].min().x();
        v = v->left;
        while(v && v->val[lower].min().x() > x) v = v->left;
        return v;
    }
    inline
    Node* stepRight(Node* v, const bool lower){
        auto x = v->val[lower].max().x();
        v = v->right;
        while(v && v->val[lower].max().x() < x) v = v->right;
        return v;
    }


    Node* find(Point key, const bool left, const bool lower){
        auto current = AVLTree<Bridges>::root;
        while(current && !isLeaf(current)){
            if(current->val[lower][!left] == key) return current;
            else if (current->val[lower].min().x() < key.x()) current = current->right;
            else current = current->left;
        }
        return nullptr;
    }

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

    bool covers(Point p, const bool lower){
        auto current = AVLTree<Bridges>::root;
        while(current){
            if(current->val[lower].min().x() <= p.x()){
                if(p.x() <= current->val[lower].max().x()){
                    return cover_comp(p,current->val[lower],lower);
                } else current = stepRight(current,lower);
            } else current = stepLeft(current,lower);
        }
        return false;
    }

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


    std::vector<Point> hullPoints(const bool lower){
        std::vector<Point> res;
        Node* e = AVLTree<Bridges>::root;
        if(!e || isLeaf(e)) return res;
        while(e){
            res.insert(res.begin(), e->val[lower].min());
            e = find(res.front(),false,lower);
        }
        e = AVLTree<Bridges>::root;
        while(e){
            res.push_back(e->val[lower].max());
            e = find(res.back(),true,lower);
        }
        if(lower) res.pop_back();
        else {
            res.erase(res.begin());
            std::reverse(res.begin(), res.end());
        }
        return res;
    }

public:
    void insert(Point p){
        AVLTree<Bridges>::insert({Bridge(p,p),Bridge(p,p)});
    }

    void remove(Point p){
        AVLTree<Bridges>::remove({Bridge(p,p),Bridge(p,p)});
    }

    bool covers(Point p){
        return covers(p,0) && covers(p,1);
    }

    std::vector<Point> upperHullPoints(){
        return hullPoints(false);
    }

    std::vector<Point> lowerHullPoints(){
        return hullPoints(true);
    }
    /*
    std::vector<std::pair<T,T>> lowerHullPoints(){
        std::vector<std::pair<T,T>> res;
        Node* e = AVLTree<Bridges<T>>::root;
        if(!e || isLeaf(e)) return res;
        while(e){
            res.insert(res.begin(), std::make_pair(e->val.lower.a.x,e->val.lower.a.y));
            e = findLower(Point(res.front().first,res.front().second),false);
        }
        e = AVLTree<Bridges<T>>::root;
        while(e){
            res.push_back(std::make_pair(e->val.lower.b.x,e->val.lower.b.y));
            e = findLower(Point(res.back().first,res.back().second),true);
        }
        res.pop_back();
        return res;
    }
     */
};
#endif //DYNAMICCONVEXHULL_CHTREE_H
