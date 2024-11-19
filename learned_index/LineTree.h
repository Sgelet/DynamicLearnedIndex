//
// Created by etoga on 5/21/24.
//

#ifndef DYNAMICCONVEXHULL_LINETREE_H
#define DYNAMICCONVEXHULL_LINETREE_H

#include <memory>
#include "../convex_hull/RankHullTree.h"
#define isLeaf this->isLeaf

template<typename NumType, typename ArithType>
struct LineNode {
    std::shared_ptr<RHTree<NumType,ArithType>> hull;
    Segment<NumType,ArithType> line;
    NumType s,t;
    uint count = 1;

    bool findLine(){
        if(!hull) return false;
        return hull->findLine(line);
    }

    LineNode(NumType val, const uint& epsilon){
        hull = std::make_shared<RHTree<NumType,ArithType>>(epsilon);
        hull->insert(val);
        hull->findLine(line);
        s = val;
        t = val;
    }

    explicit LineNode(RHTree<NumType, ArithType> *& h) {
        hull = std::shared_ptr<RHTree<NumType,ArithType>>(h);
        hull->findLine(line);
        s = (*(hull->root->min))[0].l;
        t = (*(hull->root->max))[0].r;
        count = hull->root->size;
    }

    friend bool operator<(const LineNode& lhs, const LineNode& rhs){ return lhs.t < rhs.s; }

};

template<typename NumType, typename ArithType>
class LineTree : public AVLTree<LineNode<NumType,ArithType>>{
    using Tree = AVLTree<LineNode<NumType,ArithType>>;
    using Node = typename Tree::Node;
    using LineNode = LineNode<NumType,ArithType>;
    using Hull = RHTree<NumType,ArithType>;

    uint epsilon;

protected:
    void onUpdate(Node* x){
        if(isLeaf(x)) return;
        x->val.count = x->left->val.count + x->right->val.count;
        x->val.s = x->left->val.s;
        x->val.t = x->right->val.t;
    }


public:
    explicit LineTree(const int& epsilon): epsilon(epsilon){};

    bool find(const NumType val){
        if(!Tree::root) return false;
        Node* current = Tree::root;
        while(!isLeaf(current)){
            if(val < current->right->val.s){
                current = current->left;
            } else {
                current = current->right;
            }
        }
        if(val < current->val.s) return false;
        if(current->val.t < val) return false;
        return current->val.hull->find(val);
    }

    Node* getSegment(const NumType& val, Node*& pre, Node*& succ){
        Node* current = Tree::root;

        if(!current) return nullptr;
        while(!isLeaf(current)){
            if(val < current->right->val.s){
                succ = current->right;
                current = current->left;
            } else {
                pre = current->left;
                current = current->right;
            }
        }
        if(val < current->val.s){
            succ = current;
            current = nullptr;
        } else if(current->val.t < val){
            pre = current;
            current = nullptr;
        }

        // Go-to leaves
        if(pre) while(!isLeaf(pre)) pre = pre->right;
        if(succ) while(!isLeaf(succ)) succ = succ->left;

        return current;
    }

    LineNode* getSegment(const NumType& val) {
        Node* a,*b = nullptr, *c = nullptr;
        a = getSegment(val,b,c);
        return a ? &(a->val) : nullptr;
    }

    // Inserts at a given node with pre and succ
    void insert(const NumType val, Node* current, Node* pre, Node* succ){
        Node* l = current,* r = current;
        Hull* lhull = nullptr, *rhull= nullptr;
        bool ljoined = false, lone = false, between = false, dirty_l = false, dirty_r = false;
        if(current) { // Insertion into existing segment
            dirty_l = true;
            dirty_r = true;
            lhull = current->val.hull.get();
            lhull->insert(val);
            if (!current->val.findLine()) { // Value splits line
                rhull = new Hull(epsilon);
                lhull->split({Bridge(val),Bridge(val)}, rhull);
                current->val.t = (*(lhull->root->max))[0].r;
                current->val.count = lhull->root->size;
                r = Tree::insert(LineNode(rhull),current);
                lone = true;
            } else {
                rhull = lhull;
                current->val.count++;
                current->val.s = (*(lhull->root->min))[0].l;
                current->val.t = (*(lhull->root->max))[0].r;
                Tree::retrace(current,false,false);
            }
        } else { // Insertion between segments
            if(pre) {
                l = pre;
                lhull = l->val.hull.get();
            }
            if(succ){
                r = succ;
                rhull = succ->val.hull.get();
            }
            between = true;
        }
        if(lone || between){ // Attempt to join into lhull then rhull
            if(lhull) {
                lhull->insert(val);
                if (!l->val.findLine()) lhull->remove(val);
                else {
                    dirty_l = true;
                    ljoined = true;
                    l->val.t = val;
                    l->val.count++;
                    Tree::retrace(l,false,false); // Propagate update
                }
            }
            if(rhull && !ljoined){
                rhull->insert(val);
                if (!r->val.findLine()){
                    rhull->remove(val);
                    Tree::insert(LineNode(val,epsilon),r); // No join possible, insert as single element
                } else{
                    dirty_r = true;
                    r->val.s = val;
                    r->val.count++;
                    Tree::retrace(r,false,false);
                }
            } else if(rhull && ljoined && between){
                lhull->join(rhull);
                if(!l->val.findLine()){
                    lhull->split({Bridge(r->val.s),Bridge(r->val.s)},rhull);
                    rhull->insert(r->val.s);
                } else {
                    l->val.t = r->val.t;
                    l->val.count += r->val.count;
                    Tree::retrace(l,false,false);
                    Tree::remove(r);
                }
                return;
            } else if (!ljoined && !rhull){
                Tree::insert(LineNode(val,epsilon),l);
                return;
            }
        }

        if(pre && pre != l && dirty_l){ // Attempt to join lhull with predecessor
            pre->val.hull.get()->join(lhull);
            ljoined = pre->val.findLine();
            if(!ljoined){ // Could not join - restore lhull
                pre->val.hull.get()->split({Bridge(l->val.s),Bridge(l->val.s)},lhull);
                lhull->insert(l->val.s);
            } else { // Join succeeded, update pre and remove l
                pre->val.t = l->val.t;
                pre->val.count += l->val.count;
                Tree::retrace(pre,false,false);
                if(l == r) {
                    r = pre;
                    rhull = pre->val.hull.get();
                }
                Tree::remove(l); // Deletes r, when l=r
            }
        }
        if(succ && succ != r && dirty_r){ // Attempt to join rhull with successor
            rhull->join(succ->val.hull.get());
            if(!rhull->findLine(succ->val.line)){ // Could not join - restore succ' hull
                rhull->split({Bridge(succ->val.s),Bridge(succ->val.s)},succ->val.hull.get());
                succ->val.hull->insert(succ->val.s);
            } else {
                succ->val.s = r->val.s;
                succ->val.count += r->val.count;
                std::swap(r->val.hull->root,succ->val.hull->root);
                Tree::retrace(succ,false,false);
                Tree::remove(r);
            }
        }
    }

    void insert(const NumType val){
        if(!Tree::root){
            Tree::insert(LineNode(val,epsilon));
            return;
        }

        // Find pre/post intervals
        // If they are identical, insert into interval
        // Else insert between intervals
        Node* pre = nullptr, *succ = nullptr;
        Node* current = getSegment(val, pre, succ);

        insert(val,current,pre,succ);

        //verify(Tree::root);
    }

    void remove(const NumType val, Node* current, Node* pre, Node* succ){
        // Not found
        if(!current || val < current->val.s || current->val.t < val) return;

        Node* l = current,* r = current;
        Hull* lhull, *rhull;
        bool ljoined;

        lhull = current->val.hull.get();
        lhull->remove(val);
        if(!lhull->root){ //TODO: If hull empty
            Tree::remove(current);
            // Atempt to join pre/succ
            if(!pre) return;
            if(!succ) return;
            lhull = pre->val.hull.get();
            rhull = succ->val.hull.get();
            lhull->join(rhull);
            if(!pre->val.findLine()){
                lhull->split({Bridge(succ->val.s),Bridge(succ->val.s)}, rhull);
                rhull->insert(succ->val.s);
            } else {
                pre->val.t = succ->val.t;
                pre->val.count += succ->val.count;
                Tree::retrace(pre,false,false);
                Tree::remove(succ);
            }
            return;
        }

        if (!current->val.findLine()) { // Value splits line
            rhull = new Hull(epsilon);
            lhull->split({Bridge(val),Bridge(val)}, rhull);
            current->val.t = (*(lhull->root->max))[0].r;
            current->val.count = lhull->root->size;
            r = Tree::insert(LineNode(rhull),current);
        } else {
            rhull = lhull;
            current->val.count--;
            current->val.s = (*(lhull->root->min))[0].l;
            current->val.t = (*(lhull->root->max))[0].r;
            Tree::retrace(current,false,false);
        }

        if(pre && pre != l){ // Attempt to join lhull with predecessor
            pre->val.hull.get()->join(lhull);
            ljoined = pre->val.findLine();
            if(!ljoined){ // Could not join - restore lhull
                pre->val.hull.get()->split({Bridge(l->val.s),Bridge(l->val.s)},lhull);
                lhull->insert(l->val.s);
            } else { // Join succeeded, update pre and remove l
                pre->val.t = l->val.t;
                pre->val.count += l->val.count;
                Tree::retrace(pre,false,false);
                if(l == r) {
                    r = pre;
                    rhull = pre->val.hull.get();
                }
                Tree::remove(l); // Deletes r, when l=r
            }
        }
        if(succ && succ != r){ // Attempt to join rhull with successor
            rhull->join(succ->val.hull.get());
            if(!rhull->findLine(succ->val.line)){ // Could not join - restore succ' hull
                rhull->split({Bridge(succ->val.s),Bridge(succ->val.s)},succ->val.hull.get());
                succ->val.hull->insert(succ->val.s);
            } else {
                succ->val.s = r->val.s;
                succ->val.count += r->val.count;
                std::swap(r->val.hull->root,succ->val.hull->root);
                Tree::retrace(succ,false,false);
                Tree::remove(r);
            }
        }
    }

    void remove(NumType val){
        if(!Tree::root){
            return;
        }

        // Find pre/post intervals
        // If they are identical, insert into interval
        // Else insert between intervals
        Node* pre = nullptr, *succ = nullptr, *current = getSegment(val,pre,succ);

        remove(val,current,pre,succ);
    }

    void verify(Node* v){
        if(!v) return;
        if(isLeaf(v)){
            if(v->val.s != (*(v->val.hull->root->min))[0].l) {
                std::cout << "s disagrees with hull" << std::endl;
            }
            if(v->val.t != (*(v->val.hull->root->max))[0].l) {
                std::cout << "t disagrees with hull" << std::endl;
            }
            if(v->val.count != v->val.hull->root->size) {
                std::cout << "count disagrees with hull" << std::endl;
            }
        } else {
            if(v->val.s != v->min->s) {
                std::cout << "s disagrees with min" << std::endl;
            }
            if(v->val.t != v->max->t) {
                std::cout << "t disagrees with max" << std::endl;
            }
            if(v->val.count != v->left->val.count + v->right->val.count) {
                std::cout << "count disagrees with children" << std::endl;
            }

            verify(v->left);
            verify(v->right);
        }
    }
};
#endif //DYNAMICCONVEXHULL_LINETREE_H
