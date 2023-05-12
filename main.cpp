#include <vector>
#include <random>
#include "AvlTree.h"

int main(int argc, char* argv[]){
    int n = 1000;
    auto a = std::vector<int>(n);
    for(int i=0; i<n; ++i){
        a[i] = i;
    }

    std::shuffle(a.begin(),a.end(),std::mt19937{std::random_device{}()});

    auto t = AVLTree<int>();
    for(int i=0; i<n; ++i){
        t.insert(a[i]);
        t.check();
    }
    std::shuffle(a.begin(),a.end(),std::mt19937{std::random_device{}()});
    for(int i=0; i<n; ++i){
        t.remove(a[i]);
        t.check();
    }
}