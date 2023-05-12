//
// Created by etoga on 4/11/23.
//

#ifndef DYNAMICCONVEXHULL_AVLTREE_H
#define DYNAMICCONVEXHULL_AVLTREE_H

#include <algorithm>
#include <iostream>

using uint = unsigned int;

template<class T>
class AVLTree{
    struct Node{
        Node *left = nullptr, *right = nullptr, *par = nullptr;
        uint size = 1;
        int  balance = 0;
        T val, min, max;

        Node() = default;

        explicit Node(T v) : val(v),min(v),max(v) {};

        // Deletion of entire subtree
        ~Node(){
            delete left;
            delete right;
        }
    };

    Node* root = nullptr;

public:

    ~AVLTree(){
        delete root;
    }

    void insert(T val){
        Node* parent = nullptr;
        Node* current = root;
        while(current){
            parent = current;
            if(val < current->val) current = current->left;
            else if (val > current->val) current = current->right;
            else return; // Duplicate insertion
        }
        Node* inserted = new Node(val);
        inserted->par = parent;
        if(!parent){
            root = inserted;
            return;
        }
        Node* sibling = new Node(parent->val);
        sibling->par = parent;
        if(val < parent->val){
            parent->left = inserted;
            parent->right = sibling;
        } else {
            parent->right = inserted;
            parent->left = sibling;
        }

        retrace(parent, true);
    }

    void remove(T val){
        Node* parent = nullptr;
        Node* current = root;
        while(current){
            parent = current;
            if(val < current->val) current = current->left;
            else current = current->right;
        }
        if(!parent || parent->val != val) return; // Val not found

        current = parent;
        parent = parent->par;
        Node* sibling;
        if(isLeftChild(current)) {
            sibling = parent->right;
            parent->right = nullptr;
        } else {
            sibling = parent->left;
            parent->left = nullptr;
        }
        sibling->par = parent->par;
        if(!parent->par) root = sibling;
        else if(isLeftChild(parent)) parent->par->left = sibling;
        else parent->par->right = sibling;

        delete parent;

        retrace(sibling, false);
    }

    void check(){
        checker(root);
    }

protected:
    int checker(Node *x){
        if(!x) return 0;

        auto checkL = checker(x->left);
        auto checkR = checker(x->right);

        // Checker(right) - Checker(left) == balance factor
        if(x->balance != checkR - checkL){
            std::cout << "INCORRECT BALANCE" << std::endl;
        }

        // Balance factor -1,0,1
        if(x->balance < -1 || x->balance > 1){
            std::cout << "TREE NOT BALANCED" << std::endl;
        }
        // Check sizes

        if(x->size != size(x->left)+ size(x->right) + isLeaf(x)){
            std::cout << "INCORRECT SIZE" << std::endl;
        }
        // Check min,max

        return std::max(checkL,checkR) + 1;
    }

    void rotateR(Node* x){
        Node* y = x->left;

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

        x->balance += 1 - std::min(0,y->balance);
        y->balance += 1 + std::max(0,x->balance);
    }

    void rotateL(Node* x){
        Node* y = x->right;

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

        x->balance += - 1 - std::max(0,y->balance);
        y->balance += - 1 + std::min(0,x->balance);
    }

    void rebalance(Node* x){
        if(x->balance > 0){
            if(x->right->balance < 0) rotateR(x->right);
            rotateL(x);
        } else if (x->balance < 0){
            if(x->left->balance > 0) rotateL(x->left);
            rotateR(x);
        }
    }

    void retrace(Node* x, const bool insertion){
        bool balance = true;

        for(auto current = x; current; current = current->par){
            updateData(current);
            if(!balance) continue;

            if(current->balance < -1 || current->balance > 1){
                rebalance(current);
                current = current->par;
                if(insertion || (current->balance == -1 || current->balance == 1)){
                    balance = false;
                    continue;
                }
            }

            if(current->par){
                current->par->balance += 1 - 2*(insertion == isLeftChild(current));
                if ((insertion && current->par->balance == 0) || (!insertion && (current->par->balance == -1 || current->par->balance == 1))) balance = false;
            }
        }
    }

    inline
    void updateData(Node* x){
        x->size = size(x->left) + size(x->right) + isLeaf(x);
        if(x->left) x->min = x->left->min;
        else x->min = x->val;
        if(x->right) x->max = x->right->max;
        else x->max = x->val;
    }

    inline
    bool isLeaf(const Node* x){
        return x && !(x->left || x->right);
    }

    inline
    bool isLeftChild(const Node* x){
        return (x->par && x->par->left == x);
    }

    inline
    uint size(const Node* x){
        if(x) return x->size;
        return 0;
    }



};

#endif //DYNAMICCONVEXHULL_AVLTREE_H
