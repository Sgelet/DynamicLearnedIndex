//
// Created by etoga on 5/12/23.
//

#ifndef DYNAMICCONVEXHULL_CHTREE_H
#define DYNAMICCONVEXHULL_CHTREE_H

#include <vector>
#include <CGAL/enum.h>
#include "AvlTree.h"
#include "../util.h"
#define isLeaf this->isLeaf


template<class Traits, int epsilon>
class CHTree : AVLTree<Bridges<Traits>>{
using Bridge = typename Traits::Segment_2;
using Line = typename Traits::Line_2;
using Point = typename Traits::Point_2;
using Node = typename AVLTree<Bridges<Traits>>::Node;
using Midpoint = typename Traits::Construct_midpoint_2;
using Compare_slope = typename Traits::Compare_slope_2;
using Compare_at_x = typename Traits::Compare_y_at_x_2;
using Left_turn = typename Traits::Left_turn_2;

const Midpoint midpoint = Midpoint();
const Compare_slope compare_slope = Compare_slope();
const Compare_at_x compare_at_x = Compare_at_x();
const Left_turn left_turn = Left_turn();

protected:
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
        while(v && v->val[lower].max().x() > x) v = v->left;
        return v;
    }
    inline
    Node* stepRight(Node* v, const bool lower){
        auto x = v->val[lower].max().x();
        v = v->right;
        while(v && v->val[lower].min().x() < x) v = v->right;
        return v;
    }


    // TODO: Inteded not to use stepRight/Left?
    Node* find(const Point key, const bool left, const bool lower){
        auto current = AVLTree<Bridges<Traits>>::root;
        while(current && !isLeaf(current)){
            if(current->val[lower][!left] == key) return current;
            else if (current->val[lower].min().x() < key.x()) current = current->right;
            else current = current->left;
        }
        return nullptr;
    }

    Node* hullSuccessor(const Point key, const bool lower){
        auto current = AVLTree<Bridges<Traits>>::root;
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
        auto current = AVLTree<Bridges<Traits>>::root;
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
        auto current = AVLTree<Bridges<Traits>>::root;
        while(current){
            if(current->val[lower].min() <= p){
                if(p <= current->val[lower].max()){
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
        Node* e = AVLTree<Bridges<Traits>>::root;
        if(!e || isLeaf(e)) return res;
        while(e){
            res.insert(res.begin(), e->val[lower].min());
            e = find(res.front(),false,lower);
        }
        e = AVLTree<Bridges<Traits>>::root;
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




public:
    void insert(Point p){
        AVLTree<Bridges<Traits>>::insert({Bridge(p,p),Bridge(p,p)});
    }

    void remove(Point p){
        AVLTree<Bridges<Traits>>::remove({Bridge(p,p),Bridge(p,p)});
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

    // Computes a line that intersects all segments
    // Returns true if successful, storing such a line in the parameter
    // Returns false if unsuccessful, will not modify line in this case
    bool findLine(Line& result){
        Node* x = AVLTree<Bridges<Traits>>::root; // Upper bridge - lower envelope in dual
        Node* y = AVLTree<Bridges<Traits>>::root; // Lower bridge - upper envelope in dual
        Node* x_succ, * y_pred;
        Line u,l;
        Point ul,ur,ll,lr;
        bool ul_in_l,ur_in_l,ll_in_u,lr_in_u; // Upper Left below L, ... , Lower Left above U, ...
        bool seekLeft, parallel;
        while(x && y && !isLeaf(x) && !isLeaf(y)){ // TODO: Only update moved segments
            // Get predecessors
            x_succ = hullSuccessor(x,0);
            y_pred = hullPredecessor(y,1);

            // Get the line segment on lower envelope in dual
            u = dual(x->val[0].max(),-epsilon);
            l = dual(y->val[1].min(),epsilon);

            // Determine direction of (possible) intersection
            seekLeft = slope_comp(u,l,parallel);

            if(parallel){
                //if(!ul_in_l && !ur_in_l) return false; // U and L are parallel separating lines - might never happen in rank-space
                //result = Line(x->val[0].min(),y->val[1].min()); // U and L are parallel boundaries of the shared area
                result = x->val[0].supporting_line();
                return true;
            }

            // Unbounded rays towards right
            if(isLeaf(x) && isLeaf(y)){
                if(seekLeft) return false;
                result = Line(x->val[0].max(),y->val[1].min());
                return true;
            }

            // Evaluate left endpoints
            if(!isLeaf(x_succ)){
                ur = dual(x->val[0].supporting_line(),-epsilon);
                ul = dual(x_succ->val[0].supporting_line(),-epsilon);
            } else {
                ul = Point(0,u.c(),-u.b()); // Lack of predecessor should be intersection with y-axis)
                ur = dual(x->val[0].supporting_line(),-epsilon);
            }
            if(!isLeaf(y_pred)){
                lr = dual(y->val[1].supporting_line(),epsilon);
                ll = dual(y_pred->val[1].supporting_line(),epsilon);
            } else{
                ll = Point(0,l.c(),-u.b());
                lr = dual(y->val[1].supporting_line(),epsilon);
            }

            // Determine regions
            ul_in_l = left_turn(ll,lr,ul);
            ur_in_l = left_turn(ll,lr,ur);
            ll_in_u = !left_turn(ul,ur,ll);
            lr_in_u = !left_turn(ul,ur,lr);

            if(ul_in_l != ur_in_l && ll_in_u != lr_in_u) { // Line segments intersect
                result = Line(dual(u,0),dual(l,0)); // TODO: Orientation of the line may be flipped?
                return true;
            }

            if(seekLeft){ // If slope of L strictly less than U then intersection region is left
                if(!ul_in_l) x = stepRight(x,0); // Case 1
                if(!ll_in_u) y = stepLeft(y,1); // Case 2
                if(ul_in_l && ll_in_u) { // Case 3
                    if(ur.x() > lr.x()) x = stepRight(x,0);
                    else y = stepLeft(y,1);
                }
            } else {
                if(!ur_in_l) x = stepLeft(x,0);
                if(!lr_in_u) y = stepRight(y,1);
                if(ur_in_l && lr_in_u){
                    if(ul.x() < ll.x()) x = stepLeft(x,0);
                    else y = stepRight(y,1);
                }
            }
        }
        return false;
    }
};
#endif //DYNAMICCONVEXHULL_CHTREE_H
