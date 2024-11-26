//
// Created by etoga on 4/11/23.
//

#ifndef DYNAMICCONVEXHULL_AVLTREE_H
#define DYNAMICCONVEXHULL_AVLTREE_H

#include <algorithm>

using uint = unsigned int;

template<class T, typename Comparator = std::less<T>>
class AVLTree{
    Comparator less;

public:
    struct Node{
        Node *left = nullptr, *right = nullptr, *par = nullptr;
        uint size = 1;
        int height = 1;
        T val;
        T* min = &val,* max = &val;

        explicit Node(T v) : val(v) {};


        // Deletion of entire subtree
        ~Node(){
            delete left;
            delete right;
        }

        inline
        void makeLeftChild(Node* p){
            if(!p) return;
            p->par = this;
            left = p;
        }

        inline
        void makeRightChild(Node* p){
            if(!p) return;
            p->par = this;
            right = p;
        }
    };

    Node* root = nullptr;

    ~AVLTree(){
        delete root;
    }

    Node* find(const T& val){
        Node* current = root;
        while(current && !isLeaf(current)){
            onVisit(current);
            if(less(val,*(current->right->min))) current = current->left;
            else current = current->right;
        }
        return current;
    }

    Node* insert(T val, Node* leaf){
        Node* inserted = new Node(val);
        if(!leaf){
            root = inserted;
            return inserted;
        }
        Node* parent = new Node(val);
        if(leaf->par){
            if(isLeftChild(leaf)) leaf->par->makeLeftChild(parent);
            else leaf->par->makeRightChild(parent);
        } else {
            root = parent;
        }
        inserted->par = parent;
        leaf->par = parent;
        if(less(val, leaf->val)){
            parent->left = inserted;
            parent->right = leaf;
        } else {
            parent->right = inserted;
            parent->left = leaf;
        }

        retrace(parent, true);
        return inserted;
    }

    void insert(T val){
        insert(val,find(val));
    }

    void remove(Node* leaf){
        if(!leaf) return;
        if(leaf == root){
            delete leaf;
            root = nullptr;
            return;
        }

        Node* parent = leaf->par;
        Node* sibling;
        if(isLeftChild(leaf)) sibling = parent->right;
        else sibling = parent->left;
        sibling->par = parent->par;
        if(!parent->par) root = sibling;
        else if(isLeftChild(parent)) parent->par->left = sibling;
        else parent->par->right = sibling;

        leaf->left = leaf->right = nullptr;
        delete leaf;
        parent->left = parent->right = nullptr;
        delete parent;

        onVisit(sibling);
        retrace(sibling, false);
    }

    void remove(T val){
        Node* current = find(val);
        if(!current || current->val != val) return; // Val not found
        remove(current);
    }

    void join(T k, AVLTree* r){
        Node* tl = root;
        Node* tr = r->root;
        Node* x = new Node(k);
        if(height(tl) > height(tr)+1) joinRight(root,x, r->root);
        else if(height(tr) > height(tl)+1) joinLeft(root,x,r->root);
        else {
           x->makeLeftChild(tl);
           x->makeRightChild(tr);
           updateData(x);
        }
        while(x->par) x = x->par;
        root = x;
        r->root = nullptr;
    }

    void join(Node* k, AVLTree* r) {
        Node *tl = root;
        Node *tr = r->root;
        Node *x = k;
        if (height(tl) > height(tr) + 1) joinRight(root, x, r->root);
        else if (height(tr) > height(tl) + 1) joinLeft(root, x, r->root);
        else {
            x->makeLeftChild(tl);
            x->makeRightChild(tr);
            updateData(x);
        }
        while (x->par) x = x->par;
        root = x;
        r->root = nullptr;
    }

    void join(AVLTree* r){
        Node* current = root;
        if(!current){
            root = r->root;
            r->root = nullptr;
            return;
        } else if(!r->root) return;
        T temp = *(current->max);
        split(temp, nullptr);
        join(temp,r);
    }

    // Split s.t. l < k and r > k
    // If k exists, delete it
    void split(T k, AVLTree* r){
        if(!root) return;
        Node* parent = nullptr;
        Node* v = root;
        while(v){
            parent = v;
            if(less(v->val, k)) v = v->right;
            else if(v->val == k) break;
            else v = v->left;
        }
        if(!v) v = parent;
        AVLTree tl;
        AVLTree tr;
        AVLTree temp;
        Node* current = v;
        if(v->val == k){
            current = v->par;
            if(current){
                if(isLeftChild(v)) current->left = nullptr;
                else current->right = nullptr;
            }
            tl.root = v->left;
            tr.root = v->right;
            if(tl.root) tl.root->par = nullptr;
            if(tr.root) tr.root->par = nullptr;
            v->left = nullptr;
            v->right = nullptr;
            delete v;
        }
        while(current){
            parent = current->par;
            if(parent){
                if(isLeftChild(current)) parent->left = nullptr;
                else parent->right = nullptr;
                current->par = nullptr;
            }
            if(less((*(current->right->min)), k)){
                if(current->left) current->left->par = nullptr;
                temp.root = current->left;
                temp.join(current,&tl);
                tl.join(&temp);
            } else {
                if(current->right) current->right->par = nullptr;
                temp.root = current->right;
                tr.join(current,&temp);
            }
            current = parent;
        }
        root = tl.root;
        if(r) {
            r->root = tr.root;
            tr.root = nullptr;
        }
        tl.root = nullptr;
    }

protected:

    void joinRight(Node* tl, Node* x, Node* tr){
        while(tl->right && height(tl) > height(tr) + 1) tl = tl->right;
        if(tl && tl->par) tl->par->makeRightChild(x);
        x->makeLeftChild(tl);
        x->makeRightChild(tr);
        retrace(x, true);
    }
    void joinLeft(Node* tl, Node* x, Node* tr){
        while(tr->left && height(tr) > height(tl) + 1) tr = tr->left;
        if(tr && tr->par) tr->par->makeLeftChild(x);
        x->makeLeftChild(tl);
        x->makeRightChild(tr);
        retrace(x, true);
    }

    void rotateR(Node* x){
        Node* y = x->left;
        onVisit(y);
        x->left = y->right;
        if(y->right) y->right->par = x;
        y->par = x->par;
        if(!x->par) root = y;
        else if(isLeftChild(x)) x->par->left = y;
        else x->par->right = y;
        y->right = x;
        x->par = y;

        updateData(x);
        updateData(y);
    }

    void rotateL(Node* x){
        Node* y = x->right;
        onVisit(y);
        x->right = y->left;
        if(y->left) y->left->par = x;
        y->par = x->par;
        if(!x->par) root = y;
        else if(isLeftChild(x)) x->par->left = y;
        else x->par->right = y;
        y->left = x;
        x->par = y;

        updateData(x);
        updateData(y);
    }

    void rebalance(Node* x){
        onVisit(x);
        if(height(x->right) > height(x->left)){
            if(height(x->right->right) < height(x->right->left)){
                onVisit(x->right);
                rotateR(x->right);
            }
            rotateL(x);
        } else if (height(x->right) < height(x->left)){
            if(height(x->left->right) > height(x->left->left)) {
                onVisit(x->left);
                rotateL(x->left);
            }
            rotateR(x);
        }
    }

    void retrace(Node *x, const bool insertion, bool balance = true) {
        int dif;

        for(auto current = x; current; current = current->par){
            updateData(current);
            if(!balance){
                continue;
            }

            dif = height(current->left) - height(current->right);

            if(dif < -1 || dif > 1){
                rebalance(current);
                current = current->par;
                dif = height(current->left) - height(current->right);
                if(insertion || dif == -1 || dif == 1){
                    balance = false;
                    continue;
                }
            }
            //if ((insertion && dif == 0) || (!insertion && (dif == -1 || dif == 1))) balance = false;
        }
    }

    inline
    void updateData(Node* x){
        x->size = size(x->left) + size(x->right) + isLeaf(x);
        x->height = std::max(height(x->left), height(x->right)) + 1;
        if(x->left) x->min = x->left->min;
        else x->min = &(x->val);
        if(x->right) x->max = x->right->max;
        else x->max = &(x->val);
        onUpdate(x);
    }

    virtual inline
    void onUpdate(Node* x){};

    virtual inline
    void onVisit(Node* x){};

    inline
    bool isLeaf(const Node* x){
        return x && !(x->left || x->right);
    }

    inline
    bool isLeftChild(const Node* x){
        return (x->par && x->par->left == x);
    }

    inline
    int height(const Node* x){
        if(x) return x->height;
        return 0;
    }

    inline
    uint size(const Node* x){
        if(x) return x->size;
        return 0;
    }
};

#endif //DYNAMICCONVEXHULL_AVLTREE_H
