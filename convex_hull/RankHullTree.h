//
// Created by etoga on 5/12/23.
//

#ifndef DYNAMICCONVEXHULL_RANKHULLTREE_H
#define DYNAMICCONVEXHULL_RANKHULLTREE_H

#include <array>
#include <vector>
#include <numeric>
#include <stack>
#include "AvlTree.h"
#define isLeaf this->isLeaf

template<typename T>
T gcd(T a, T b){

}

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
        S c = std::gcd(n,d);
        n/=c;
        d/=c;
        //auto count = std::min(__builtin_ctz(n),__builtin_ctz(d));
        //n = n >> count;
        //d = d >> count;
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

    friend Quotient operator+(const Quotient& lhs, const Quotient& rhs){
        Quotient res = lhs;
        res += rhs;
        return res;
    }

    friend Quotient operator-(const Quotient& lhs, const Quotient& rhs){
        Quotient res = lhs;
        res -= rhs;
        return res;
    }

    friend Quotient operator*(const Quotient& lhs, const Quotient& rhs){
        Quotient res = lhs;
        res *= rhs;
        return res;
    }

    friend Quotient operator/(const Quotient& lhs, const Quotient& rhs){
        Quotient res = lhs;
        res /= rhs;
        return res;
    }

    friend bool operator==(const Quotient& lhs, const Quotient& rhs){ return lhs.n * rhs.d == rhs.n * lhs.d; }
    friend bool operator<(const Quotient& lhs, const Quotient& rhs){ return lhs.n * rhs.d < rhs.n * lhs.d; }
    friend bool operator>(const Quotient& lhs, const Quotient& rhs){ return rhs < lhs; }
    friend bool operator<=(const Quotient& lhs, const Quotient& rhs){ return !(lhs > rhs); }
    friend bool operator>=(const Quotient& lhs, const Quotient& rhs){ return !(lhs < rhs); }

};

template<typename NT>
struct Bridge{
    NT l,r; // Left and Right values
    uint ldiff,rdiff; // Difference in rank to median

    explicit Bridge(NT x): l(x),r(x),ldiff(0),rdiff(0){};

    Bridge(NT l, NT r, uint ldiff, uint rdiff): l(l), r(r), ldiff(ldiff), rdiff(rdiff){};

    friend bool operator<(const Bridge& lhs, const Bridge& rhs){ return lhs.l < rhs.l; }
    friend bool operator==(const Bridge& lhs, const Bridge& rhs){ return lhs.l == rhs.l && lhs.r == rhs.r; }
};

template<typename NT>
using Bridges = std::array<Bridge<NT>,2>;


template<typename NT>
struct Point{
    NT x,y;

    Point() = default;

    Point(NT x, NT y): x(x),y(y){};

    Point(NT& x, NT& y): x(x),y(y){};
};

template<typename NT, typename AT>
inline
AT getSlope(const Point<NT>& l, const Point<NT>& r){
    if(l.x == r.x) return AT(0);
    else return (AT(r.y)-AT(l.y))/(AT(r.x)-AT(l.x));
}

template<typename NT, typename AT>
struct Segment{
    Point<NT> l,r;
    AT slope;

    Segment()= default;

    Segment(const Bridge<NT>& b, NT offset) : Segment(b,offset,0){}

    // TODO: y_offset might be overflown unsigned value
    Segment(const Bridge<NT>& b, const NT& x_offset, const NT& y_offset) : Segment(
                Point<NT>(x_offset-b.ldiff, b.l + y_offset),
                Point<NT>(x_offset+b.rdiff, b.r + y_offset)
                ){}

    Segment(const Point<NT> l, const Point<NT> r): l(l),r(r){
        slope = getSlope<NT,AT>(l,r);
    };

    // TODO: Does not work for q left of l if T unsigned
    inline
    Point<AT> eval(AT q) const {
        return Point<AT>(q, AT(l.y) + slope * (AT(q)-(AT(l.x))));
    }

    inline
    Point<AT> midpoint() const {
        return eval(AT(l.x)+(AT(r.x)-AT(l.x)) * AT(1)/AT(2));
    }
};

template<typename NT, typename AT>
class RHTree : public AVLTree<Bridges<NT>>{
using AVL = AVLTree<Bridges<NT>>;
using Node = typename AVL::Node;
using Bridge = Bridge<NT>;
using Segment = Segment<NT,AT>;


private:
    int epsilon = 0;

protected:
    // TODO: Computations done in NT not in AT
    inline
    bool left_turn(const Point<NT>& a, const Point<NT>& b, const Point<NT>& c){
        return (a.x - c.x)*(b.y-c.y) > (b.x - c.x)*(a.y - c.y);
    }

    inline
    bool right_turn(const Point<NT>& a, const Point<NT>& b, const Point<NT>& c){
        return (a.x - c.x)*(b.y-c.y) < (b.x - c.x)*(a.y - c.y);
    }

    template<bool lower>
    inline
    bool cover_comp(const Point<NT>& p, const Point<NT>& l, const Point<NT>& r){
        if(lower) return !right_turn(l,r,p);
        else return !left_turn(l,r,p);
    }

    template<bool lower>
    inline
    bool strict_comp(const Point<NT>& p, const Point<NT>& l, const Point<NT>& r){
        if(lower) return left_turn(l,r,p);
        else return right_turn(l,r,p);
    }

    inline
    bool direction_comp(const Segment& l, const Segment& r, bool& parallel){
        parallel = (l.slope == r.slope);
        return l.slope < r.slope;
    }

    template<bool lower>
    inline
    bool slope_comp(const AT& l, const AT& r){
        if(l == r) return true;
        else return (l < r) != lower;
    }

    template<bool lower>
    inline
    bool m_comp(const Segment& l, const Segment& r, const AT& q){
        if(l.slope == AT(0)) return lower;
        if(r.slope == AT(0)) return !lower;
        Point<AT> lm = l.eval(q);
        Point<AT> rm = r.eval(q);
        if(lm.y== rm.y) return true;
        return (lm.y < rm.y) == lower;
    }

    template<bool lower>
    Bridge findBridge(Node* v){
        Node* x = v->left;
        Node* y = v->right;
        Segment e_l, e_r;
        Point<AT> l_m,r_m;
        bool undecided,dirty_l = true, dirty_r = true;
        uint v_rank = 1 + AVL::size(x), l_rank = 1 + AVL::size(x->left), r_rank = 1 + AVL::size(x) + AVL::size(y->left);

        AT m = AT(x->size) + AT(1)/AT(2);
        AT lr;

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
            lr = (AT(r_m.y) - AT(l_m.y))/(AT(r_m.x) - AT(l_m.x));
            if (!isLeaf(x) && slope_comp<lower>(e_l.slope,lr)){
                stepLeft<lower>(x,l_rank); undecided = false; dirty_l = true;
            }
            if (!isLeaf(y) && slope_comp<lower>(lr,e_r.slope)) {
                stepRight<lower>(y,r_rank); undecided = false; dirty_r = true;
            }
            if (undecided) {
                if (!isLeaf(x) && m_comp<lower>(e_l,e_r,m) || isLeaf(y)) {
                    stepRight<lower>(x,l_rank); dirty_l = true;
                } else {
                    stepLeft<lower>(y,r_rank); dirty_r = true;
                }
            }
        }
        return Bridge(x->val[lower].l,y->val[lower].r,v_rank - l_rank, r_rank - v_rank);
    }

    void onUpdate(Node* x){
        if(isLeaf(x)) return;
        x->val[0] = findBridge<false>(x);
        x->val[1] = findBridge<true>(x);
    }

    template<bool lower>
    inline
    NT stepLeft(Node*& v){
        NT x = v->val[lower].l;
        v = v->left;
        while(v && v->val[lower].r > x) v = v->left;
        return x;
    }

    template<bool lower>
    inline
    NT stepLeft(Node*& v, uint& rank){
        NT x = rank - v->val[lower].ldiff;
        v = v->left;
        while(v){
            rank -= AVL::size(v->right) + isLeaf(v);
            if(rank + v->val[lower].rdiff <= x) break;
            v = v->left;
        }
        return x;
    }

    template<bool lower>
    inline
    NT stepRight(Node*& v){
        NT x = v->val[lower].r;
        v = v->right;
        while(v && v->val[lower].l < x) v = v->right;
        return x;
    }

    template<bool lower>
    inline
    NT stepRight(Node*& v, uint& rank){
        NT x = rank + v->val[lower].rdiff;
        v = v->right;
        while(v){
            rank += AVL::size(v->left);
            if(rank - v->val[lower].ldiff >= x) break;
            v = v->right;
        }
        return x;
    }

    template<bool lower>
    inline
    uint stepLeftHull(Node*& v, uint& rank, const NT& bound){
        Node* t = nullptr;
        uint x = rank - v->val[lower].ldiff;
        stepLeft<lower>(v,rank);

        while(v != t){ // Comparison between x and y
            t = v;
            if(v && rank + v->val[lower].rdiff > x) stepLeft<lower>(v,rank);
            if(v && rank - v->val[lower].ldiff < bound) stepRight<lower>(v,rank);
        }

        return x;
    }

    template<bool lower>
    inline
    uint stepRightHull(Node*& v, uint& rank, const NT& bound){
        Node* t = nullptr;
        uint x = rank + v->val[lower].rdiff;
        stepRight<lower>(v,rank);

        while(v != t){
            t = v;
            if(v && rank - v->val[lower].ldiff < x) stepRight<lower>(v,rank);
            if(v && rank + v->val[lower].rdiff > bound) stepLeft<lower>(v,rank);
        }

        return x;
    }

    template<bool lower>
    Node* hullSuccessor(const NT& key){
        Node* current = AVL::root;
        NT min;
        while(current){
            min = current->val[lower].l;
            if(min == key) break;
            else if (min < key) stepRight<lower>(current);
            else stepLeft<lower>(current);
        }
        return current;
    }
    template<bool lower>
    Node* hullSuccessor(const Node* p){
        return hullSuccessor<lower>(p->val[lower].r);
    }

    template<bool lower>
    Node* hullPredecessor(const NT& key){
        Node* current = AVL::root;
        NT max;
        while(current){
            max = current->val[lower].r;
            if(max == key) break;
            else if (max < key) stepRight<lower>(current);
            else stepLeft<lower>(current);
        }
        return current;
    }

    template<bool lower>
    Node* hullPredecessor(const Node* p){
        return hullPredecessor<lower>(p->val[lower].l);
    }

    // Computes the segment whose right endpoint has rank r
    template<bool lower>
    Segment segmentHullPredecessor(const uint& r){
        Node* current = AVL::root;
        uint v = 1 + AVL::size(current->left);
        uint x;

        while(!isLeaf(current)){
            x = v + current->val[lower].rdiff;
            if(x == r) break;
            if(x < r) stepRight<lower>(current,v);
            else stepLeft<lower>(current,v);
        }

        return Segment(current->val[lower], v, lower ? epsilon : -epsilon);
    }

    // Computes the segment whose left endpoint has rank r
    template<bool lower>
    Segment segmentHullSuccessor(const uint& r){
        Node* current = AVL::root;
        uint v = 1 + AVL::size(current->left);
        uint x;

        while(!isLeaf(current)){
            x = v - current->val[lower].ldiff;
            if(x == r) break;
            if(x < r) stepRight<lower>(current,v);
            else stepLeft<lower>(current,v);
        }

        return Segment(current->val[lower],v, lower ? epsilon : -epsilon);
    }

    // TODO: Consider function to translate rank to left and right point
    template<bool lower>
    bool covers(Point<NT> p){
        Node* current = AVL::root;
        uint rank = 1 + AVL::size(AVL::root->left);
        while(current){
            if(current->val[lower].l <= p){
                if(p <= current->val[lower].max()){
                    return cover_comp<lower>(p,{rank - current->val[lower].ldiff, current->val[lower].l},{rank + current->val[lower].rdiff, current->val[lower].r});
                } else stepRight<lower>(current,rank);
            } else stepLeft<lower>(current,rank);
        }
        return false;
    }

    template<bool lower>
    bool strict_covers(Point<NT> p){
        Node* current = AVL::root;
        uint rank = 1 + AVL::size(AVL::root->left);
        while(current){
            if(rank - current->val[lower].ldiff <= p.x){
                if(p.x <= rank + current->val[lower].rdiff){
                    return strict_comp<lower>(p,{rank - current->val[lower].ldiff, current->val[lower].l},{rank + current->val[lower].rdiff, current->val[lower].r});
                } else stepRight<lower>(current,rank);
            } else stepLeft<lower>(current,rank);
        }
        return false;
    }

    template<bool lower>
    std::vector<NT> hullPoints(){
        std::vector<NT> res;
        Node* e = AVL::root;
        if(!e) return res;
        res.push_back((*(e->min))[lower].l);
        while(!isLeaf(e)){
            e = hullSuccessor<lower>(res.back());
            res.push_back(e->val[lower].r);
        }
        res.pop_back();
        if(!lower) std::reverse(res.begin(), res.end());
        return res;
    }


public:
    explicit RHTree(const int& epsilon): epsilon(epsilon){};

    void insert(NT x){
        AVL::insert({Bridge(x),Bridge(x)});
    }

    void remove(NT x){
        AVL::remove({Bridge(x),Bridge(x)});
    }

    bool find(NT x){
        Node* found = AVL::find({Bridge(x),Bridge(x)});
        return found && found->val[0].l == x;
    }

    void join(RHTree<NT,AT>* r){
        Node* tl = AVL::root, *tr = r->root;
        if(!tr) return;
        else if (!tl){
            AVL::root = tr;
            r->root = nullptr;
            return;
        }
        Node* x = new Node(tr->val);
        if(AVL::height(tl) > AVL::height(tr) + 1) AVL::joinRight(tl,x,tr);
        else if (AVL::height(tr) > AVL::height(tl) + 1) AVL::joinLeft(tl, x, tr);
        else {
            x->makeLeftChild(tl);
            x->makeRightChild(tr);
            AVL::updateData(x);
        }
        while (x->par) x = x->par;
        AVL::root = x;
        r->root = nullptr;
    }

    void split(const Bridges<NT>& k, RHTree<NT,AT>* r){
        Node* parent;
        Node* v = AVL::find(k);
        RHTree<NT,AT> tl = RHTree(epsilon);
        RHTree<NT,AT> tr = RHTree(epsilon);
        RHTree<NT,AT> temp = RHTree(epsilon);
        Node* current = v;
        if(v->val == k){
            current = current->par;
            if(current){
                if(AVL::isLeftChild(v)) tr.root = current->right;
                else tl.root = current->left;
            }
            delete v;
        } else {
            if(v->val < k) while(current->par->par && !AVL::isLeftChild(current)) current = current->par;
            else while(current->par->par && AVL::isLeftChild(current)) current = current->par;
            current = current->par;
            tl.root = current->left;
            tr.root = current->right;
        }

        if(tl.root) tl.root->par = nullptr;
        if(tr.root) tr.root->par = nullptr;
        if(current) {
            current->left = nullptr;
            current->right = nullptr;
        }
        while(current){
            parent = current->par;
            if(parent){ // Disconnects but wants to compare
                if(AVL::isLeftChild(current)) {
                    parent->right->par = nullptr;
                    temp.root = parent->right;
                    tr.join(&temp);
                } else {
                    parent->left->par = nullptr;
                    temp.root = parent->left;
                    temp.join(&tl);
                    std::swap(tl.root,temp.root);
                }
                parent->left = nullptr;
                parent->right = nullptr;
            }
            delete current;
            current = parent;
        }
        AVL::root = tl.root;
        if(r) {
            r->root = tr.root;
            tr.root = nullptr;
        }
        tl.root = nullptr;
    }

    bool covers(Point<NT> p){
        return covers<false>(p) && covers<true>(p);
    }

    std::vector<NT> upperHullPoints(){
        return hullPoints<false>();
    }

    std::vector<NT> lowerHullPoints(){
        return hullPoints<true>();
    }

    // Line verification
    bool verifyNoLine(){
        Node* x;
        std::stack<Node*> s;
        uint rank = 1;
        for(s.push(AVL::root);!s.empty();){ // Iterative in-order traversal
            x = s.top();
            s.pop();
            if(!isLeaf(x)) {
                if(x->right) s.push(x->right);
                if(x->left) s.push(x->left);
                continue;
            }
            if(strict_covers<false>({rank,x->val[1].l+2*epsilon})) return true;
            if(strict_covers<true>({rank,x->val[0].l-2*epsilon})) return true;

            rank++;
        }

        return false;
    }

    bool verifyLine(const Segment& l){
        Node* x;
        std::stack<Node*> s;
        uint rank = 1;
        for(s.push(AVL::root);!s.empty();){
            x = s.top();
            s.pop();
            if(!isLeaf(x)) {
                if(x->right) s.push(x->right);
                if(x->left) s.push(x->left);
                continue;
            }
            if(right_turn(l.l,l.r, {rank, x->val[0].l + epsilon})) return false;
            if(left_turn(l.l, l.r, {rank, x->val[0].l - epsilon})) return false;

            rank++;
        }

        return true;
    }

    template<bool uwedge>
    int witnessCheck(const Segment& b, const Segment& a1, const Segment& a2){
        const bool qr = uwedge ? left_turn(a1.l,a1.r,b.r) : left_turn(a2.r,a2.l,b.l);
        const bool ql = uwedge ? left_turn(a2.l,a2.r,b.l) : left_turn(a1.r,a1.l,b.r);

        if(qr && ql) return 0;

        if(ql){
            if(right_turn(uwedge ? b.l : b.r, uwedge ? b.r : b.l, a1.r)){
                if(uwedge) return 1;
                else return -1;
            } else if(!slope_comp<false>(a1.slope,b.slope)){
                if(uwedge) return -1;
                else return 1;
            }
            return 0;
        }
        if(qr){
            if(right_turn(uwedge ? b.l : b.r, uwedge ? b.r : b.l, a1.r)){
                if(uwedge) return -1;
                else return 1;
            } else if(!slope_comp<false>(b.slope, a2.slope)){
                if(uwedge) return 1;
                else return -1;
            }
            return 0;
        }

        if(!slope_comp<false>(a1.slope, b.slope)){
            if(uwedge) return -1;
            else return 1;
        } else if(!slope_comp<false>(b.slope,a2.slope)){
            if(uwedge) return 1;
            else return -1;
        } else {
            return 0;
        }
    }

    template<bool uwedge>
    Segment findWitness(const uint r, Node*& x, uint v, uint ub, uint lb){
        // From r extract Segments a1, a2
        Segment a1 = segmentHullPredecessor<uwedge>(r);
        Segment a2 = segmentHullSuccessor<uwedge>(r);
        Segment b;

        // Check for non-wedge
        if(a1.l.x == a1.r.x){
            return a2;
        } else if(a2.l.x == a2.r.x){
            return a1;
        }

        int direction;

        while(ub != lb){
            b = Segment(x->val[!uwedge], v, uwedge ? -epsilon: epsilon);

            direction = witnessCheck<uwedge>(b,a1,a2);

            if(direction < 0){
                ub = stepLeftHull<!uwedge>(x,v,lb);
            } else if (direction > 0){
                lb = stepRightHull<!uwedge>(x,v,ub);
            } else {
                return b; // Needs to be shiftee?
            }
        }

        // Double wedge scenario
        Segment b1 = segmentHullPredecessor<!uwedge>(ub);
        Segment b2 = segmentHullSuccessor<!uwedge>(lb);

        if(b1.l.x == b1.r.x){
            return b2;
        } else if(b2.l.x == b2.r.x){
            return b1;
        }

        if(!uwedge){ // Enforce a on u and b on l
            std::swap(a1,b1);
            std::swap(a2,b2);
        }

        if(witnessCheck<true>(b1,a1,a2) == 0) return b1;
        else if(witnessCheck<true>(b2,a1,a2) == 0) return b2;
        else if(witnessCheck<false>(a1,b1,b2) == 0) return a1;
        else return a2;
    }

    // Computes a line that intersects all segments
    // Returns true if successful, storing such a line in the parameter
    // Returns false if unsuccessful, will not modify line in this case
    bool findLine(Segment& res){
        Node* x = AVL::root; // Upper bridge - lower envelope in dual
        Node* y = AVL::root; // Lower bridge - upper envelope in dual
        uint u_rank = 1 + AVL::size(x->left);
        uint l_rank = u_rank;
        Segment u,l;
        bool ul_in_l,ur_in_l,ll_in_u,lr_in_u; // Upper Left below L, ... , Lower Left above U, ...
        bool seekLeft, parallel;
        bool dirty_x = true, dirty_y = true;

        uint uub = AVL::size(x);
        uint ulb = 1;
        uint lub = uub;
        uint llb = ulb;

        while(uub != ulb && lub != llb) {
            if(!(x && y)) return false; // Insufficient precision

            // Get the line segment
            if (dirty_x) u = Segment(x->val[1], u_rank, epsilon);
            if (dirty_y) l = Segment(y->val[0], l_rank, -epsilon);

            // Reset
            dirty_x = false;
            dirty_y = false;

            // Determine direction of (possible) intersection
            seekLeft = direction_comp(l, u, parallel);

            // Determine regions
            ul_in_l = right_turn(l.l, l.r, u.l);
            ur_in_l = right_turn(l.l, l.r, u.r);
            ll_in_u = left_turn(u.l, u.r, l.l);
            lr_in_u = left_turn(u.l, u.r, l.r);

            if (parallel) { // Either witness or multiple stepping
                if (!ur_in_l) { // u and l are both witnesses
                    res = u;
                    return true;
                }

                if (l.r.x < u.l.x) { // l lies left of u
                    uub = stepLeftHull<true>(x, u_rank, ulb);
                    llb = stepRightHull<false>(y, l_rank, lub);
                } else if (l.l.x > u.r.x) { // l lies right of u
                    ulb = stepRightHull<true>(x, u_rank, uub);
                    lub = stepLeftHull<false>(y, l_rank, llb);
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
                    uub = stepLeftHull<true>(x, u_rank, ulb);
                    dirty_x = true;
                }
                if (!ll_in_u) { // Case 2: l is free to move
                    lub = stepLeftHull<false>(y, l_rank, llb);
                    dirty_y = true;
                }
                if (ul_in_l && ll_in_u) { // Case 3: step rightmost to left
                    if (l.l.x > u.r.x) { // l lies right of u
                        lub = stepLeftHull<false>(y, l_rank, llb);
                        dirty_y = true;
                    } else if (l.r.x < u.l.x) { // u lies right of l
                        uub = stepLeftHull<true>(x, u_rank, ulb);
                        dirty_x = true;
                    } else { // overlap is evidence of intersection
                        return false;
                    }
                }
            } else {
                if (!ur_in_l) {
                    ulb = stepRightHull<true>(x, u_rank, uub);
                    dirty_x = true;
                }
                if (!lr_in_u) {
                    llb = stepRightHull<false>(y, l_rank, lub);
                    dirty_y = true;
                }
                if (ur_in_l && lr_in_u) { // step leftmost to right
                    if (l.r.x < u.l.x) { // l lies left of u
                        llb = stepRightHull<false>(y, l_rank, lub);
                        dirty_y = true;
                    } else if (l.l.x > u.r.x) { // u lies left of l
                        ulb = stepRightHull<true>(x, u_rank, uub);
                        dirty_x = true;
                    } else { // overlap is evidence of intersection
                        return false;
                    }
                }
            }
        }

        // No intersection found, search for witness
        if(uub == ulb) res = findWitness<true>(u_rank,y,l_rank,lub,llb);
        else res = findWitness<false>(l_rank,x,u_rank,uub,ulb);

        return true;
    }
};
#endif //DYNAMICCONVEXHULL_RANKHULLTREE_H
