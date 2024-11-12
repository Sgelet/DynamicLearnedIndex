//
// Created by etoga on 5/12/23.
//

#ifndef DYNAMICCONVEXHULL_CHTREE_H
#define DYNAMICCONVEXHULL_CHTREE_H

#include <vector>
#include <CGAL/enum.h>
#include <queue>
#include "AvlTree.h"
#include "../util.h"
#define isLeaf this->isLeaf


template<class Traits, int epsilon>
class CHTree : AVLTree<CBridges<Traits>>{
using Bridge = typename Traits::Segment_2;
using Line = typename Traits::Line_2;
using Point = typename Traits::Point_2;
using Node = typename AVLTree<CBridges<Traits>>::Node;
using Midpoint = typename Traits::Construct_midpoint_2;
using Compare_slope = typename Traits::Compare_slope_2;
using Compare_at_x = typename Traits::Compare_y_at_x_2;
using Orientation = typename Traits::Orientation_2;

static constexpr Midpoint midpoint = Midpoint();
static constexpr Compare_slope compare_slope = Compare_slope();
static constexpr Compare_at_x compare_at_x = Compare_at_x();
static constexpr Orientation orientation = Orientation();

protected:
    bool slope_comp(const Bridge& l, const Bridge& r, const bool lower){
        CGAL::Comparison_result res = compare_slope(l,r);
        if(res == CGAL::EQUAL) return true;
        return (res == CGAL::SMALLER) != lower;
    }

    bool direction_comp(const Bridge& l, const Bridge& r, bool& parallel){
        CGAL::Comparison_result res = compare_slope(l,r);
        parallel = res == CGAL::EQUAL;
        return res == CGAL::SMALLER;
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
        if(lower) return !right_turn(b.min(),b.max(),p);
        else return !left_turn(b.min(),b.max(),p);
    }

    bool strict_cover_comp(const Point& p, const Bridge& b, const bool lower){
        if(lower) return left_turn(b.min(),b.max(),p);
        else return right_turn(b.min(),b.max(),p);
    }

    inline
    bool left_turn(const Point& p, const Point& q, const Point& r){
        return orientation(p,q,r) == CGAL::LEFT_TURN;
    }

    inline
    bool right_turn(const Point&p, const Point& q, const Point& r){
        return orientation(p,q,r) == CGAL::RIGHT_TURN;
    }

    Bridge findBridge(Node* v, const bool lower){
        Node* x = v->left;
        Node* y = v->right;
        Bridge e_l, e_r, lr;
        bool undecided;

        Point m = midpoint((*(x->max))[lower].max(),(*(y->min))[lower].min());

        while (!(isLeaf(x) && isLeaf(y))) {
            undecided = true;
            e_l = x->val[lower];
            e_r = y->val[lower];
            lr = Bridge(midpoint(e_l),midpoint(e_r));
            if (!isLeaf(x) && slope_comp(e_l,lr,lower)){
                stepLeft(x,lower); undecided = false;
            }
            if (!isLeaf(y) && slope_comp(lr,e_r,lower)) {
                stepRight(y,lower); undecided = false;
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
    Node* stepUp(Node* v, const bool lower){
        auto min = v->val[lower].min().x();
        auto max = v->val[lower].max().x();
        v = v->par;
        while(v && v->val[lower].min() < min && max < v->val[lower].max()) v = v->par;
        return v;
    }

    inline
    auto stepLeft(Node*& v, const bool& lower){
        auto x = v->val[lower].min();
        v = v->left;
        while(v && v->val[lower].max().x() > x.x()) v = v->left;
        return x;
    }
    inline
    auto stepRight(Node*& v, const bool& lower){
        auto x = v->val[lower].max();
        v = v->right;
        while(v && v->val[lower].min().x() < x.x()) v = v->right;
        return x;
    }

    auto stepLeftHull(Node*& v, const Point& bound, const bool& lower){
        Node* t = nullptr;
        auto x = v->val[lower].min();
        v = v->left;

        while(v != t) {
            t = v;
            while (v && v->val[lower].max().x() > x.x()) v = v->left;
            while (v && v->val[lower].min().x() < bound.x()) v = v->right;
        }
        return x;
    }
    auto stepRightHull(Node*& v, const Point& bound, const bool& lower){
        Node* t = nullptr;
        auto x = v->val[lower].max();
        v = v->right;

        while(v != t) {
            t = v;
            while (v && v->val[lower].min().x() < x.x()) v = v->right;
            while (v && v->val[lower].max().x() > bound.x()) v = v->left;
        }
        return x;
    }

    // TODO: Inteded not to use stepRight/Left?
    Node* find(const Point key, const bool left, const bool lower){
        auto current = AVLTree<CBridges<Traits>>::root;
        while(current && !isLeaf(current)){
            if(current->val[lower][!left] == key) return current;
            else if (current->val[lower].min().x() < key.x()) current = current->right;
            else current = current->left;
        }
        return nullptr;
    }

    Node* hullSuccessor(const Point key, const bool lower){
        auto current = AVLTree<CBridges<Traits>>::root;
        Point min;
        while(current){
            min = current->val[lower].min();
            if(min == key) break;
            else if (min < key) stepRight(current,lower);
            else stepLeft(current,lower);
        }
        if(!current){
            std::cout << key.x() << "," << key.y() << std::endl;
        }
        return current;
    }

    Node* hullSuccessor(const Node* p, const bool lower){
        return hullSuccessor(p->val[lower].max(), lower);
    }
    Node* hullPredecessor(const Point key, const bool lower){
        auto current = AVLTree<CBridges<Traits>>::root;
        Point max;
        while(current){
            max = current->val[lower].max();
            if(max == key) break;
            else if (max < key) stepRight(current,lower);
            else stepLeft(current,lower);
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
        auto current = AVLTree<CBridges<Traits>>::root;
        while(current){
            if(current->val[lower].min() <= p){
                if(p <= current->val[lower].max()){
                    return cover_comp(p,current->val[lower],lower);
                } else stepRight(current,lower);
            } else stepLeft(current,lower);
        }
        return false;
    }

    bool strict_covers(Point p, const bool lower){
        auto current = AVLTree<CBridges<Traits>>::root;
        while(current){
            if(current->val[lower].min() <= p){
                if(p <= current->val[lower].max()){
                    return strict_cover_comp(p,current->val[lower],lower);
                } else stepRight(current,lower);
            } else stepLeft(current,lower);
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
        Node* e = AVLTree<CBridges<Traits>>::root;
        if(!e || isLeaf(e)) return res;
        while(e){
            res.insert(res.begin(), e->val[lower].min());
            e = find(res.front(),false,lower);
        }
        e = AVLTree<CBridges<Traits>>::root;
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
        return Line(p.hx(),-1/p.hw(),-p.hy()-offset/p.hw());
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
        AVLTree<CBridges<Traits>>::insert({Bridge(p,p),Bridge(p,p)});
    }

    void remove(Point p){
        AVLTree<CBridges<Traits>>::remove({Bridge(p,p),Bridge(p,p)});
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

    inline
    Bridge shift(const Bridge& b,const int offset){
        return Bridge{shift(b.min(),offset),shift(b.max(),offset)};
    }

    inline
    Point shift(const Point& p, const int ox, const int oy){
        return Point(p.x()+ox,p.y()+oy);
    }

    inline
    Point shift(const Point& p,const int offset){
        return Point(p.x(),p.y()+offset);
    }

    bool verifyNoLine(const Node* v){
        if(isLeaf(v)) return strict_covers(shift(v->val[1].min(),epsilon + epsilon),0) || strict_covers(shift(v->val[0].min(),-2*epsilon),1);
        return verifyNoLine(v->left) || verifyNoLine(v->right);
    }

    bool verifyNoLine(){
        return verifyNoLine(this->root);
    }


    bool verifyNoLineIter(){
        Node* x;
        std::queue<Node*> q;
        for(q.push(this->root);!q.empty();q.pop()){
            x = q.front();
            if(!isLeaf(x)) {
                if(x->left) q.push(x->left);
                if(x->right) q.push(x->right);
                continue;
            }
            if(strict_covers(shift(x->val[1].min(),2*epsilon),0)) return true;
            if(strict_covers(shift(x->val[0].min(),-2*epsilon),1)) return true;
        }

        return false;
    }

    bool verifyLine(const Bridge& b){
        Node* x;
        std::queue<Node*> q;
        for(q.push(this->root);!q.empty();q.pop()){
            x = q.front();
            if(!isLeaf(x)) {
                if(x->left) q.push(x->left);
                if(x->right) q.push(x->right);
                continue;
            }
            if(right_turn(b.min(),b.max(), shift(x->val[0].min(),epsilon))) return false;
            if(left_turn(b.min(),b.max(), shift(x->val[0].min(),-epsilon))) return false;
            //if(cover_comp(shift(x->val[1].min(),epsilon),b,false)) return false;
        }

        return true;
    }

    template<bool uwedge>
    bool findWitness(){

    }

    // 0 is avoids
    // -1 left
    // 1 right
    int witnessCheck(const Point& b1, const Point& b2, const Point& a1, const Point& a2, const Point& a3, const bool& uwedge){
        const bool qr = left_turn(a1,a2,b2);
        const bool ql = left_turn(a2,a3,b1);

        if(qr && ql){
            return 0;
        }
        if(ql){
            if(right_turn(b1,b2,a2)){
                if(uwedge) return 1;
                else return -1;
            } else if(!slope_comp(Bridge(a1,a2),Bridge(b1,b2),false)){
                if(uwedge) return -1;
                else return 1;
            }
            return 0;
        }
        if(qr){
            if(right_turn(b1,b2,a2)){
                if(uwedge) return -1;
                else return 1;
            } else if(!slope_comp(Bridge(b1,b2),Bridge(a2,a3),false)){
                if(uwedge) return 1;
                else return -1;
            }
            return 0;
        }

        // Decide if stepping left or right depending on slope?
        if(!slope_comp(Bridge(a1,a2),Bridge(b1,b2),false)){
            if(uwedge) return -1;
            else return 1;
        } else if(!slope_comp(Bridge(b1,b2),Bridge(a2,a3),false)){
            if(uwedge) return 1;
            else return -1;
        }else {
            return 0;
        }
    }

    // Computes a line that intersects all segments
    // Returns true if successful, storing such a line in the parameter
    // Returns false if unsuccessful, will not modify line in this case
    bool findLine(Bridge& res) {
        Node *x = this->root; // Upper bridge - lower chain
        Node *y = this->root; // Lower bridge - upper chain
        Bridge u, l; // Segments on upper/lower chain
        bool ul_in_l, ur_in_l, ll_in_u, lr_in_u; // Upper Left below L, ... , Lower Left above U, ...
        bool seekLeft, parallel; // Decision variables
        bool dirty_x = true, dirty_y = true; // Update variables
        auto uub = x->max->data[1].max();
        auto ulb = x->min->data[1].min();
        auto lub = y->max->data[0].max();
        auto llb = y->min->data[0].min();

        while (uub != ulb && lub != llb) { // TODO: Only update moved segments
            // Get the line segment
            if (dirty_x) u = shift(x->val[1], epsilon);
            if (dirty_y) l = shift(y->val[0], -epsilon);

            // Reset
            dirty_x = false;
            dirty_y = false;

            // Determine direction of (possible) intersection
            seekLeft = direction_comp(l, u, parallel);

            // Determine regions
            ul_in_l = right_turn(l.min(), l.max(), u.min());
            ur_in_l = right_turn(l.min(), l.max(), u.max());
            ll_in_u = left_turn(u.min(), u.max(), l.min());
            lr_in_u = left_turn(u.min(), u.max(), l.max());

            if (parallel) { // Either witness or multiple stepping
                if (!ur_in_l) { // u and l are both witnesses
                    res = Bridge(u.min(), u.max());
                    return true;
                }

                if (l.max().x() < u.min().x()) { // l lies left of u
                    uub = stepLeftHull(x, ulb, 1);
                    llb = stepRightHull(y, lub, 0);
                } else if (l.min().x() > u.max().x()) { // l lies right of u
                    ulb = stepRightHull(x, uub, 1);
                    lub = stepLeftHull(y, llb, 0);
                } else { // l and u overlap - existence of intersection follows
                    return false;
                }
                dirty_x = true;
                dirty_y = true;
                continue;
            } else if (ul_in_l != ur_in_l && ll_in_u != lr_in_u) { // Line segments intersect
                return false;
            }

            if (seekLeft) { // If slope of l strictly less than u then intersection region is left
                if (!ul_in_l) { // Case 1: u is free to move
                    uub = stepLeftHull(x, ulb, 1);
                    dirty_x = true;
                }
                if (!ll_in_u) { // Case 2: l is free to move
                    lub = stepLeftHull(y, llb, 0);
                    dirty_y = true;
                }
                if (ul_in_l && ll_in_u) { // Case 3: step rightmost to left
                    if(l.min().x() > u.max().x()){ // l lies right of u
                        lub = stepLeftHull(y, llb, 0);
                        dirty_y = true;
                    } else if(l.max().x() < u.min().x()){ // u lies right of l
                        uub = stepLeftHull(x, ulb, 1);
                        dirty_x = true;
                    } else { // overlap is evidence of intersection
                        return false;
                    }
                }
            } else {
                if (!ur_in_l) {
                    ulb = stepRightHull(x, uub, 1);
                    dirty_x = true;
                }
                if (!lr_in_u) {
                    llb = stepRightHull(y, lub, 0);
                    dirty_y = true;
                }
                if (ur_in_l && lr_in_u) { // step leftmost to right
                    if(l.max().x() < u.min().x()){ // l lies left of u
                        llb = stepRightHull(y, lub, 0);
                        dirty_y = true;
                    } else if(l.min().x() > u.max().x()){ // u lies left of l
                        uub = stepRightHull(x, ulb, 1);
                        dirty_x = true;
                    } else { // overlap is evidence of intersection
                        return false;
                    }
                }
            }
        }

        // No intersection found, search for witness
        const bool uwedge = ulb == uub;
        int direction;
        auto a2 = uwedge ? ulb : llb;
        auto a1 = hullPredecessor(a2,uwedge)->val[uwedge].min();
        auto a3 = hullSuccessor(a2,uwedge)->val[uwedge].max();
        auto b1 = u.min(); // Unnecessary assigns
        auto b2 = u.max();

        if(a1 == a2){
            res = shift(Bridge(a2,a3), uwedge ? epsilon : -epsilon);
            return true;
        } else if (a2 == a3){
            res = shift(Bridge(a1,a2), uwedge ? epsilon : -epsilon);
            return true;
        }

        if(!uwedge){
            std::swap(a1,a3); // relabel pre/succ for symmetry
        }

        if(uwedge) x = y; //this->root;
        if(uwedge) ulb = llb;
        if(uwedge) uub = lub;

        // {1, 2, 3, 5, 6, 7, 9, 11, 12, 13, 14}
        while(ulb != uub){ // Double-stepping into wedge still can cause issues - see the data in comment above
            u = shift(x->val[!uwedge], uwedge ? -2*epsilon : 2*epsilon);
            b1 = u.min();
            b2 = u.max();

            if(!uwedge) std::swap(b1,b2);

            direction = witnessCheck(b1,b2,a1,a2,a3,uwedge);

            if(direction < 0){
                uub = stepLeftHull(x, ulb, !uwedge);
            } else if (direction > 0){
                ulb = stepRightHull(x, uub, !uwedge);
            } else {
                res = shift(x->val[!uwedge], uwedge ? -epsilon : epsilon);
                return true;
            }

            // Avoiding slopes seem somehow plausible, but this generates counterexamples
            /*
            if(uwedge) std::swap(b1,b2);

            if(right_turn(b1,b2,a3)) ulb = stepRight(x,!uwedge);
            else if(left_turn(b2,b1,a1)) uub = stepLeft(x,!uwedge);
            else {
                res = shift(x->val[!uwedge], uwedge ? -epsilon : epsilon);
                return true;
            }*/
        }

        if(!uwedge){
            std::swap(a1,a3); // relabel pre/succ for symmetry
        }

        // Double wedge scenario
        a1 = shift(a1, uwedge ? epsilon : -epsilon);
        a2 = shift(a2, uwedge ? epsilon : -epsilon);
        a3 = shift(a3, uwedge ? epsilon : -epsilon);

        b2 = shift(uub, uwedge ? -epsilon : epsilon);
        b1 = shift(hullPredecessor(uub,!uwedge)->val[!uwedge].min(), uwedge ? -epsilon : epsilon);
        auto b3 = shift(hullSuccessor(uub,!uwedge)->val[!uwedge].max(), uwedge ? -epsilon : epsilon);

        if(b1 == b2){
            res = Bridge(b2,b3);
            return true;
        } else if (b2 == b3){
            res = Bridge(b1,b2);
            return true;
        }

        if(!uwedge){ // Enforce a on u and b on l
            std::swap(a1,b1);
            std::swap(a2,b2);
            std::swap(a3,b3);
        }

        if(witnessCheck(a2,a1,b3,b2,b1,false) == 0) res = Bridge(a1,a2);
        else if(witnessCheck(a3,a2,b3,b2,b1,false) == 0) res = Bridge(a2,a3);
        else if(witnessCheck(b1,b2,a1,a2,a3,true) == 0) res = Bridge(b1,b2);
        else res = Bridge(b2,b3);

        //std::cout << "we wedging and edging" << std::endl;
        return true;
    }
};
#endif //DYNAMICCONVEXHULL_CHTREE_H
