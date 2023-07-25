//
// Created by etoga on 5/12/23.
//

#ifndef DYNAMICCONVEXHULL_CHTREE_H
#define DYNAMICCONVEXHULL_CHTREE_H

#include <vector>
#include "AvlTree.h"
#include "util.h"
#define isLeaf this->isLeaf

template<class T>
class CHTree : AVLTree<Bridges<T>>{
using Node = typename AVLTree<Bridges<T>>::Node;

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
        Node* x = v->left;
        Node* y = v->right;
        Line e_l, e_r, lr;
        T lx, rx, m;
        Point<T> l,r;
        bool undecided;
        m = avgX(x->max.upper.b,y->min.upper.a);
        while (!(isLeaf(x) && isLeaf(y))) {
            undecided = true;
            e_l = Line(x->val.upper.a,x->val.upper.b);
            e_r = Line(y->val.upper.a,y->val.upper.b);
            lx = avgX(x->val.upper.a,x->val.upper.b);
            rx = avgX(y->val.upper.a,y->val.upper.b);
            l = {lx,e_l.eval(lx)};
            r = {rx,e_r.eval(rx)};
            lr = Line(l,r);
            if (e_l.slope <= lr.slope && !isLeaf(x)){
                x = stepLeftUpper(x); undecided = false;
            }
            if (lr.slope <= e_r.slope && !isLeaf(y)) {
                y = stepRightUpper(y); undecided = false;
            }
            if (undecided) {
                auto a = e_l.eval(m);
                auto b = e_r.eval(m);
                if (!isLeaf(x) && a >= b || isLeaf(y)) {
                    x = x->right;
                } else {
                    y = y->left;
                }
            }
        }
        return {x->val.upper.a,y->val.upper.b};
    }

    Bridge<T> findLowerBridge(Node* v){
        Node* x = v->left;
        Node* y = v->right;
        Line e_l, e_r, lr;
        T lx, rx, m;
        Point<T> l,r;
        bool undecided;
        m = avgX(x->max.lower.b,y->min.lower.a);
        while (!(isLeaf(x) && isLeaf(y))) {
            undecided = true;
            e_l = Line(x->val.lower.a,x->val.lower.b);
            e_r = Line(y->val.lower.a,y->val.lower.b);
            lx = avgX(x->val.lower.a,x->val.lower.b);
            rx = avgX(y->val.lower.a,y->val.lower.b);
            l = {lx,e_l.eval(lx)};
            r = {rx,e_r.eval(rx)};
            lr = Line(l,r);
            if (e_l.slope >= lr.slope && !isLeaf(x)){
                x = stepLeftLower(x); undecided = false;
            }
            if (lr.slope >= e_r.slope && !isLeaf(y)) {
                y = y->right; undecided = false;
            }
            if (undecided) {
                auto a = e_l.eval(m);
                auto b = e_r.eval(m);
                if (!isLeaf(x) && a <= b || isLeaf(y)) {
                    x = x->right;
                } else {
                    y = y->left;
                }
            }
        }
        return {x->val.lower.a,y->val.lower.b};
    }


    void onUpdate(Node* x){
        if(isLeaf(x)) return;
        x->val.upper = findUpperBridge(x);
        x->val.lower = findLowerBridge(x);
    }

    // TODO: Step right and test
    inline
    Node* stepLeftUpper(Node* v){
        auto x = v->val.upper.a.x;
        v = v->left;
        while(v && v->val.upper.a.x > x) v = v->left;
        return v;
    }
    inline
    Node* stepRightUpper(Node* v){
        auto x = v->val.upper.b.x;
        v = v->right;
        while(v && v->val.upper.a.x < x) v = v->right;
        return v;
    }
    inline
    Node* stepLeftLower(Node* v){
        auto x = v->val.lower.a.x;
        v = v->left;
        while(v && v->val.lower.a.x > x) v = v->left;
        return v;
    }
    inline
    Node* stepRightLower(Node* v){
        auto x = v->val.lower.b.x;
        v = v->right;
        while(v && v->val.lower.a.x < x) v = v->right;
        return v;
    }

    Node* findUpper(Point<T> key, bool left){
        auto current = AVLTree<Bridges<T>>::root;
        while(current && !isLeaf(current)){
            if((left && current->val.upper.a == key) || (!left && current->val.upper.b == key)) return current;
            else if (current->val.upper.a.x < key.x) current = current->right;
            else current = current->left;
        }
        return nullptr;
    }

    Node* findLower(Point<T> key, bool left){
        auto current = AVLTree<Bridges<T>>::root;
        while(current && !isLeaf(current)){
            if((left && current->val.lower.a == key) || (!left && current->val.lower.b == key)) return current;
            else if (current->val.lower.a.x < key.x) current = current->right;
            else current = current->left;
        }
        return nullptr;
    }

    bool coversUpper(Point<T> p){
        auto current = AVLTree<Bridges<T>>::root;
        while(current){
            if(current->val.upper.a.x <= p.x){
                if(p.x <= current->val.upper.b.x){
                    // Check
                    auto l = Line(current->val.upper.a,current->val.upper.b);
                    return p.y <= l.eval(p.x);
                } else current = stepRightUpper(current);
            } else current = stepLeftUpper(current);
        }
        return false;
    }
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
public:
    void insert(T x, T y){
        AVLTree<Bridges<T>>::insert(Bridges(x,y));
    }

    void remove(T x, T y){
        AVLTree<Bridges<T>>::remove(Bridges(x,y));
    }

    bool covers(T x, T y){
        auto p = Point(x,y);
        return coversUpper(p) && coversLower(p);
    }

    std::vector<std::pair<T,T>> upperHullPoints(){
        std::vector<std::pair<T,T>> res;
        Node* e = AVLTree<Bridges<T>>::root;
        if(!e || isLeaf(e)) return res;
        while(e){
            res.insert(res.begin(), std::make_pair(e->val.upper.a.x,e->val.upper.a.y));
            e = findUpper(Point(res.front().first,res.front().second),false);
        }
        e = AVLTree<Bridges<T>>::root;
        while(e){
            res.push_back(std::make_pair(e->val.upper.b.x,e->val.upper.b.y));
            e = findUpper(Point(res.back().first,res.back().second),true);
        }
        res.erase(res.begin());
        std::reverse(res.begin(), res.end());
        return res;
    }

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
};
#endif //DYNAMICCONVEXHULL_CHTREE_H
