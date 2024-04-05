#include <vector>
#include <random>
#include <chrono>
#include "LearnedIndex.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

using hrc = std::chrono::high_resolution_clock;
void runtimeTest(uint size){
    // Read data
    std::mt19937 engine(42);
    std::vector<uint> data;
    data.reserve(size);
    for(uint i = 1; i<size; ++i){
        data.push_back(i);
    }

    std::shuffle(data.begin(),data.end(),engine);

    auto index = LearnedIndex<K,uint,64>();

    std::cout << "Inserting"<<std::endl;

    auto t0 = hrc::now();
    volatile bool b;
    for(unsigned int i : data){
        b = index.insert(i);
    }
    auto t1 = hrc::now();

    std::cout << "Insertion done after "<< (t1 - t0).count() * 1e-9 << "s and b is " << b << std::endl;
    /*
    std::shuffle(data.begin(),data.end(),engine);

    t0 = hrc::now();
    t1 = hrc::now();
    acc = t1 - t0;
    std::cout << "Removing"<<std::endl;
    for(size_t i=0; i<data.size()/window_size; i++) {
        t0 = hrc::now();
        for (size_t j = i * window_size; j < (i + 1) * window_size; j++) {
            CH.remove(data[j].first, data[j].second);
        }
        t1 = hrc::now();
        acc += t1 - t0;
    }
     */
}

int main(int argc, char* argv[]){
    runtimeTest(400000u);
}