#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include "LearnedIndex.h"

using hrc = std::chrono::high_resolution_clock;

void runtimeTest(uint elements, uint lines){
    // Read data
    std::mt19937 engine(42);
    std::vector<int> data;
    data.reserve(elements*lines);
    int base = 8;
    for(int k = 0; k<lines; ++k){
        for(int i=1; i<=elements; ++i) {
            data.push_back((k*elements*512)+i*base);
        }
        base *= 2;
    }

    std::shuffle(data.begin(),data.end(),engine);

    auto index = LearnedIndex<int>();

    std::cout << "Inserting"<<std::endl;

    auto t0 = hrc::now();
    volatile bool b;
    int count = 0;
    for(int i : data){
        count++;
        std::cout << count << " ";
        b = index.insert(i);
    }
    auto t1 = hrc::now();

    //std::cout << "Insertion done after "<< (t1 - t0).count() * 1e-9 << "s and b is " << b << std::endl;
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
    runtimeTest(1000,6);
}