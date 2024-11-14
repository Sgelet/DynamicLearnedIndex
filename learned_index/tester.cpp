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

    int count = 0;
    auto index = LearnedIndex<int64_t>();

    std::cout << "Inserting"<<std::endl;

    auto t0 = hrc::now();
    for(int i : data){
        count++;

        index.insert(i);
        for(int j=0; j<count; ++j){
            if(!index.find(data[j])){
                std::cout << "cannot find recently inserted" << std::endl;
            }
        }
        index.verify();
    }
    auto t1 = hrc::now();

    std::cout << "Insertion done after "<< (t1 - t0).count() * 1e-9 << std::endl;

    std::shuffle(data.begin(),data.end(),engine);
    count = 0;

    std::cout << "Removing"<<std::endl;

    t0 = hrc::now();
    for(int i: data) {
        count++;

        index.remove(i);

        for(int j=count; j<data.size(); ++j){
            if(!index.find(data[j])){
                std::cout << "cannot find old inserted " << data[j] << std::endl;
            }
        }

        index.verify();
    }
    t1 = hrc::now();
    std::cout << "Removal done after "<< (t1 - t0).count() * 1e-9 << std::endl;
}

int main(int argc, char* argv[]){
    runtimeTest(10000,2);
}