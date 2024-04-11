#include <vector>
#include <random>
#include <chrono>
#include <cstring>
#include "RankHullTree.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/convex_hull_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2;

unsigned int reversibleHash(unsigned int x){
    x*=0xDEADBEEF;
    x=x^(x>>17);
    x*=0x01234567;
    x+=0x88776655;
    x=x^(x>>4);
    x=x^(x>>9);
    x*=0x91827363;
    x=x^(x>>7);
    x=x^(x>>11);
    x=x^(x>>20);
    x*=0x77773333;
    return x;
}

template<typename T>
bool verify(RHTree<T,8>& CH, std::vector<T>& data, int size){
    std::vector<T> out_rank;
    std::vector<Point_2> out_cgal;
    std::vector<Point_2> data_2;
    std::vector<T> sorted_data;
    sorted_data.reserve(size);
    sorted_data.assign(data.begin(),data.begin()+size);
    std::sort(sorted_data.begin(), sorted_data.end());
    data_2.reserve(size);
    for(int i=0; i<size; ++i){
        data_2.emplace_back(i,sorted_data[i]);
    }

    CGAL::lower_hull_points_2(data_2.begin(), data_2.end(), std::back_inserter(out_cgal));
    out_rank = CH.lowerHullPoints();

    for(int i=0; i<out_cgal.size(); ++i){
       if(out_rank[i] != out_cgal[i].y()) {
           return false;
       }
    }

    out_cgal.clear();
    out_rank.clear();

    CGAL::upper_hull_points_2(data_2.begin(), data_2.end(), std::back_inserter(out_cgal));
    out_rank = CH.upperHullPoints();

    for(int i=0; i<out_cgal.size(); ++i){
        if(out_rank[i] != out_cgal[i].y()) {
            return false;
        }
    }

    return true;
}

template<typename T>
bool verificationTest(int verify_step, bool shuffle){
    std::mt19937 engine(42);

    std::vector<T> data;
    T x;
    // while(std::cin >> x) data.push_back(x);
    for(int i = 0; i<1000000; ++i){
        data.push_back(reversibleHash(i));
    }

    if(shuffle) std::shuffle(data.begin(), data.end(), engine);

    RHTree<T,8> CH;

    std::cout << "Data processed, beginning verification test \nInsertion: ";

    int acc = 0;
    bool success = true;
    for(auto iter = data.begin(); iter != data.end(); ++iter){
        CH.insert(*iter);

        if(++acc % verify_step == 0) if(!verify(CH,data,acc)) {
            success = false;
            break;
        }
    }

    std::cout << success << std::endl;
    if(!success){
        return false;
    }
    std::cout << "Deletion: ";
    success = true;
    for(auto riter = data.rbegin(); riter != data.rend(); ++riter){
        CH.remove(*riter);
        if(--acc % verify_step == 0) if(!verify(CH,data,acc)) {
            success = false;
            break;
        }
    }
    std::cout << success << std::endl;
    return success;
}

bool find_line(){
    auto CH = RHTree<uint, 1>();

    std::vector<uint> easy = {1,2,3,4,5,6};
    std::cout << "Found easy line: " << CH.findLine() << std::endl;

    CH = RHTree<uint,1>();
    std::vector<uint> hard = {2,5,6,7,9};
    for(auto e: hard) CH.insert(e);
    std::cout << "Found hard line: " << CH.findLine() << std::endl;

    CH.insert(15);
    std::cout << "Identified impossible: " << !CH.findLine() << std::endl;

    return true;
}

int main(int argc, char* argv[]){
    if(argc > 1){
        return verificationTest<unsigned long>(atoi(argv[1]),true);
    }

    return find_line();
}


