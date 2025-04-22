#ifndef LEARNEDINDEX_H
#define LEARNEDINDEX_H

#include <unordered_map>
#include <iostream>
#include "LineTree.h"

/*
template<class NumType>
struct less{
    bool operator()(const typename Traits::Segment_2& lhs, const NumType& rhs) const {
        return lhs.max().y() < rhs;
    }
};*/

#ifndef STORAGE
#define STORAGE true
#endif

#ifndef OPT
#define OPT false
#endif

template<typename NumType>
struct Page{
    int prev = -1, succ = -1;
    std::vector<NumType> data;

    Page(NumType v, int prev, int succ) : prev(prev), succ(succ) {
        data.emplace_back(v);
    }

    void insert(NumType v) {
        data.insert(std::lower_bound(data.begin(), data.end(), v), v);
    }

    bool remove(NumType v){
        auto iter = std::lower_bound(data.begin(),data.end(), v);
        if(iter != data.end() && *iter == v) data.erase(iter);

        return data.empty();
    }
};


template<typename NumType, int epsilon = 255, typename ArithType = Quotient<NumType>>
struct LearnedIndex {
    using Segment = LineNode<NumType,epsilon,ArithType>;
    using Node = typename AVLTree<Segment>::Node;
    using Page = ::Page<NumType>;

    int min_index = -1;
    std::unordered_map<NumType,uint> h;
    std::vector<Page> pages;
    LineTree<NumType,epsilon,ArithType> lineTree = LineTree<NumType,epsilon,ArithType>();

    LearnedIndex()= default;

protected:
    int index(NumType v){
        uint min = -1;
        int res = -1;
        NumType id = v/epsilon;
        for(int i=-2; i<=2; i++){
            auto search = h.find(id+i);
            if(search != h.end()){
                NumType t = pages[search->second].data.front();
                uint diff = std::max(v,t) - std::min(v,t);
                if(diff < min){
                    min = diff;
                    res = search->second;
                }
            }
        }
        return res;
    }

    int getPage(const NumType& v, const Segment* f){
        if(!f) return -1;
        int i;
        if(v >= f->t) i = index(f->t);
        else if(v <= f->s) i = index(f->s);
        else {
            NumType a = floor(f->line.x_at_y(ArithType(v)));
            if(a == NumType(0)) i = index(f->s);
            else i = index(floor(f->line.y_at_x(ArithType(a))));
        }

        while(i >= 0 && pages[i].data.front()/epsilon > v/epsilon) i = pages[i].prev;
        while(i >= 0 && pages[i].succ >= 0 && pages[pages[i].succ].data.front()/epsilon < v/epsilon) i = pages[i].succ;

        return i;
    }

    Node* getPredSeg(const NumType& val, Node*& prev, Node*& succ){
        return lineTree.getSegment(val,prev,succ);
    }

public:
    bool runPages(){
        long succ_seen = -1;
        for(uint i=0; i<pages.size(); ++i){
            if(pages[i].prev == -1 && i != min_index){
                std::cout << "missing prev" << std::endl;
                return false;
            }
            if(pages[i].succ == -1){
                if(succ_seen < 0) succ_seen = i;
                else{
                    std::cout << "too many succ missing" << std::endl;
                    return false;
                }
            }
        }

        return true;
    }

    void insert(NumType val){
        Node* p = nullptr, *s = nullptr, *c = getPredSeg(val,p,s);
        if(c && c->val.hull->find(val)) return; // Element exists
        Segment* f = c ? &(c->val) : p ? &(p->val) : s ? &(s->val) : nullptr;
        int i = -1,succ = -1;
        NumType id = val/epsilon;

        if constexpr (STORAGE && !OPT) {
            i = getPage(val, f);
        }

        lineTree.insert(val,c,p,s); // Needs to happen after getPage

        if constexpr (!STORAGE || OPT) return;

        if(i >= 0){
            succ = pages[i].succ;
            if( pages[i].data.front()/epsilon == id) {
                pages[i].insert(val);
                return;
            }
            if(succ >= 0 && pages[succ].data.front()/epsilon == id){
                pages[succ].insert(val);
                return;
            }
        }
        int j = pages.size();
        if(min_index < 0 || pages[min_index].data.front() > val){
            succ = min_index;
            min_index = j;
        }
        pages.emplace_back(val,i,succ);
        if(i >= 0) pages[i].succ = j;
        if(succ >= 0) pages[succ].prev = j;
        h[id] = j;
    }

    void remove(NumType val){
        Node* p = nullptr,* s = nullptr, *c = getPredSeg(val,p,s);
        if(!c) return; // Value not found
        Segment* f = &(c->val);

        if constexpr (STORAGE && !OPT) {
            int i = getPage(val, f);
            if(i==-1) return;

            if (pages[i].data.front() / epsilon != val / epsilon){
                i = pages[i].succ;
            }

            if (pages[i].remove(val)) {
                h.erase(val / epsilon);
                if (i == min_index) min_index = pages[i].succ;
                if (pages[i].prev >= 0) pages[pages[i].prev].succ = pages[i].succ;
                if (pages[i].succ >= 0) pages[pages[i].succ].prev = pages[i].prev;
                int j = pages.size() - 1;
                if (i != j) {
                    h[pages[j].data.front() / epsilon] = i;
                    if (min_index == j) min_index = i;
                    if (pages[j].prev >= 0) pages[pages[j].prev].succ = i;
                    if (pages[j].succ >= 0) pages[pages[j].succ].prev = i;
                    std::swap(pages[i], pages[j]);
                }
                pages.pop_back();
            }
        }
        lineTree.remove(val,c,p,s);
    }

    bool find(NumType val){
        int i = getPage(val,lineTree.getSegment(val));
        for(;i != -1 && pages[i].data.front() <= val; i=pages[i].succ) {
            auto search = std::lower_bound(pages[i].data.begin(), pages[i].data.end(), val);
            if (search != pages[i].data.end()) return val == *search;
        }
        return false;
    }

    std::vector<NumType> range(const NumType& lo, const NumType& hi) {
        int i = getPage(lo,lineTree.getSegment(lo));
        int j;
        std::vector<NumType> res;
        for(;i != -1; i=pages[i].succ){
            for(auto& v: pages[i].data) {
                if (v < lo) continue;
                else if (v > hi) return res;
                res.emplace_back(v);
            }
        }
        return res;
    }

    size_t size_in_bytes(){
        size_t res = sizeof(*this);
        res += sizeof(NumType) * h.size(); // Approximate size of hashmap
        res += sizeof(Page) * pages.capacity();
        if(lineTree.root) res += sizeof(NumType)*lineTree.root->val.count;
        res += lineTree.size_in_bytes();
        return res;
    }

    int segments_count() {
        if(!lineTree.root) return 0;
        else return lineTree.root->size;
    }
};
#endif //LEARNEDINDEX_H
