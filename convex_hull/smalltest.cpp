#include <vector>
#include <random>
#include <chrono>
#include <cstring>
#include "RankHullTree.h"
#include "CHTree.h"

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
    return x >> 10;
}

template<typename NT, typename AT>
bool verify(RHTree<NT,AT>& CH, std::vector<NT>& data, int size){
    std::vector<NT> out_rank;
    std::vector<Point_2> out_cgal;
    std::vector<Point_2> data_2;
    std::vector<NT> sorted_data;
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

template<typename NT, typename AT>
bool verificationTest(int verify_step, bool shuffle){
    std::mt19937 engine(42);

    std::vector<NT> data;
    NT x;
    // while(std::cin >> x) data.push_back(x);
    for(int i = 0; i<10000; ++i){
        data.push_back(i+100000);
    }

    if(shuffle) std::shuffle(data.begin(), data.end(), engine);

    RHTree<NT,AT> CH(1);

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

void printListOfPoints(const std::vector<uint>& data, uint offset){
    std::cout << "{";
    std::cout << "(" << 1 << "," << data[0]+offset << ")";
    for(int i=1;i<data.size();++i){
        std::cout << ",(" << i+1 << "," << data[i]+offset << ")";
    }
    std::cout<<"}"<<std::endl;
}

void printListOfPoints(const std::vector<uint>& data){
    printListOfPoints(data,-4);
    printListOfPoints(data,4);
}

bool find_line(const int repititions){
    bool success = true;

    std::srand(42);
    for(int i=2; i<repititions; i++) {
        auto CH = CHTree<K,4>();
        auto RH = RHTree<int,Quotient<int>>(4);


        K::Segment_2 line;
        Segment<int,Quotient<int>> rank_line;

        uint rank = 1;
        uint val = 4+(rand() % 4);//rand()&(1<<16);
        std::vector<uint> data;
        bool found;
        bool found_rank;

        data.push_back(val);
        CH.insert({rank, val});
        RH.insert(val);

        do {
            rank++;
            val += 1 + (rand() % 4);
            data.push_back(val);
            CH.insert({rank, val});
            RH.insert(val);

            found = CH.findLine(line);
            found_rank = RH.findLine(rank_line);



            if (found && !CH.verifyLine(line)) {
                std::cout << "ERROR: Found but failed verify: [i,rank] = ["<<i<<","<<rank<<"]"<< std::endl;
                printListOfPoints(data);
                success = false;
            }
            if (found == CH.verifyNoLineIter()){
                std::cout << "ERROR: Line disagrees with verification" << std::endl;
                printListOfPoints(data);
                success = false;
            }
            if (found_rank && !RH.verifyLine(rank_line)) {
                std::cout << "ERROR: Rank-Found but failed verify: [i,rank] = ["<<i<<","<<rank<<"]"<< std::endl;
                printListOfPoints(data);
                success = false;
            }
            if (found_rank == RH.verifyNoLine()){
                std::cout << "ERROR: Rank-Line disagrees with verification" << std::endl;
                printListOfPoints(data);
                success = false;
            }
            if(found != found_rank){
                std::cout << "RANKERROR" << std::endl;
                printListOfPoints(data);
                success = false;
            }
        } while (!CH.verifyNoLine());
        //std::cout << "Failed to find at:\n";
        //for (auto e: data) std::cout << e << " ";
        //std::cout << std::endl;
    }
    /*

    auto RH = RHTree<uint, 1>();
    auto CH = CHTree<K,1>();
    K::Segment_2 old_line,new_line;

    std::vector<uint> data = {1,2};
    for(int i=0; i<data.size(); ++i) {
        RH.insert(data[i]);
        CH.insert({i,data[i]});
    }
    //std::cout << "Found trivial line: " << CH.findLine(old_line) << std::endl;
    //CH.findLine(old_line);
    std::cout << "Found trivial line (new): " << CH.findLine(new_line) << std::endl;
    std::cout << "Passed verification: " << CH.verifyLine(new_line) << std::endl;
    if(!CH.verifyLine(new_line)) printListOfPoints(data);

    RH = RHTree<uint,1>();
    CH = CHTree<K,1>();
    data = {1,2,4,5};
    //std::vector<uint> data = {1,2};
    for(int i=0; i<data.size(); ++i) {
        RH.insert(data[i]);
        CH.insert({i,data[i]});
    }
    //std::cout << "Found easy line: " << CH.findLine(old_line) << std::endl;
    //CH.findLine(old_line);
    std::cout << "Found easy line (new): " << CH.findLine(new_line) << std::endl;
    std::cout << "Passed verification: " << CH.verifyLine(new_line) << std::endl;
    CH.findLine(new_line);
    if(!CH.verifyLine(new_line)) printListOfPoints(data);

    RH = RHTree<uint,1>();
    CH = CHTree<K,1>();
    data = {1,2,4,5,6,7,8};
    //std::vector<uint> data = {1,2};
    for(int i=0; i<data.size(); ++i) {
        RH.insert(data[i]);
        CH.insert({i,data[i]});
    }
    //std::cout << "Found easy line extended: " << CH.findLine(old_line) << std::endl;
    //CH.findLine(old_line);
    std::cout << "Found easy line extended (new): " << CH.findLine(new_line) << std::endl;
    std::cout << "Passed verification: " << CH.verifyLine(new_line) << std::endl;
    CH.findLine(new_line);
    if(!CH.verifyLine(new_line)) printListOfPoints(data);

    RH = RHTree<uint,1>();
    CH = CHTree<K,1>();
    data = {3,4,5,7,11};
    for(int i=0; i<data.size(); ++i) {
        RH.insert(data[i]);
        CH.insert({i,data[i]});
    }
    //std::cout << "Found hard line: " << CH.findLine(old_line) << std::endl;
    //CH.findLine(old_line);
    std::cout << "Found hard line (new): " << CH.findLine(new_line) << std::endl;
    std::cout << "Passed verification: " << CH.verifyLine(new_line) << std::endl;
    CH.findLine(new_line);
    if(!CH.verifyLine(new_line)) printListOfPoints(data);

    RH.insert(15);
    CH.insert({data.size(),15});
    //data = {1,2,4,5,6,7,8,10,12,13,14,16,17,19,20,21,23,25,27,29};
    //std::cout << "Identified impossible: " << !CH.findLine(old_line) << std::endl;
    //CH.findLine(old_line);
    std::cout << "Identified impossible (new): " << !CH.findLine(new_line) << std::endl;
    std::cout << "Failed verification: " << !CH.verifyLine(new_line) << std::endl;
    std::cout << "VerifyNoLine: " << CH.verifyNoLineIter() << std::endl;
    CH.findLine(new_line);

    CH = CHTree<K,1>();
    data = {1, 3, 4, 6, 7, 8, 10, 12, 14, 15, 16, 17, 18, 20, 22, 24, 25, 26, 27, 29, 30, 32, 33, 34, 35};
    for(int i=0; i<data.size(); ++i) {
        RH.insert(data[i]);
        CH.insert({i,data[i]});
    }
    //std::cout << "Long boi: " << CH.findLine(old_line) << std::endl;
    //CH.findLine(old_line);
    std::cout << "Long boi (new): " << CH.findLine(new_line) << std::endl;
    std::cout << "Passed verification: " << CH.verifyLine(new_line) << std::endl;
    CH.findLine(new_line);
    if(!CH.verifyLine(new_line)) printListOfPoints(data);

    CH = CHTree<K,1>();
    data = {1,2,4,5,6,7,8,10,12,13,14,16,17};
    for(int i=0; i<data.size(); ++i) {
        RH.insert(data[i]);
        CH.insert({i,data[i]});
    }
    std::cout << "Opposite wedge (new): " << CH.findLine(new_line) << std::endl;
    std::cout << "Passed verification: " << CH.verifyLine(new_line) << std::endl;
    CH.findLine(new_line);
    if(!CH.verifyLine(new_line)) printListOfPoints(data);


    CH = CHTree<K,1>();
    data = {1,2,4,5,6,7,9,11,13};
    for(int i=0; i<data.size(); ++i) {
        RH.insert(data[i]);
        CH.insert({i,data[i]});
    }
    std::cout << "Initial wedge (new): " << CH.findLine(new_line) << std::endl;
    std::cout << "Passed verification: " << CH.verifyLine(new_line) << std::endl;
    CH.findLine(new_line);
    if(!CH.verifyLine(new_line)) printListOfPoints(data);

    CH = CHTree<K,1>();
    data = {1, 3, 5, 7, 8, 9, 11, 12, 14, 16, 18, 20, 21, 23, 25};
    for(int i=0; i<data.size(); ++i) {
        RH.insert(data[i]);
        CH.insert({i,data[i]});
    }
    std::cout << "Reverse intersection (new): " << CH.findLine(new_line) << std::endl;
    std::cout << "Passed verification: " << CH.verifyLine(new_line) << std::endl;
    CH.findLine(new_line);
    if(!CH.verifyLine(new_line)) printListOfPoints(data);

    CH = CHTree<K,1>();
    data = {1, 2, 3, 5, 7, 9};
    for(int i=0; i<data.size(); ++i) {
        RH.insert(data[i]);
        CH.insert({i,data[i]});
    }
    std::cout << "Reverse intersection 2 (new): " << CH.findLine(new_line) << std::endl;
    std::cout << "Passed verification: " << CH.verifyLine(new_line) << std::endl;
    CH.findLine(new_line);
    if(!CH.verifyLine(new_line)) printListOfPoints(data);

    CH = CHTree<K,1>();
    data = {1, 3, 4, 5, 6, 7, 9, 11, 12, 14, 15, 16};
    for(int i=0; i<data.size(); ++i) {
        RH.insert(data[i]);
        CH.insert({i,data[i]});
    }
    std::cout << "Wrong wedge (new): " << CH.findLine(new_line) << std::endl;
    std::cout << "Passed verification: " << CH.verifyLine(new_line) << std::endl;
    CH.findLine(new_line);
    if(!CH.verifyLine(new_line)) printListOfPoints(data);

    CH = CHTree<K,1>();
    data = {1, 2, 4, 6, 8, 10, 12, 14, 15, 17, 19, 20, 21, 22, 23};
    for(int i=0; i<data.size(); ++i) {
        RH.insert(data[i]);
        CH.insert({i,data[i]});
    }
    std::cout << "Troublemaker (new): " << !CH.findLine(new_line) << std::endl;
    std::cout << "Passed verification: " << !CH.verifyLine(new_line) << std::endl;
    std::cout << "VerifyNoLine: " << CH.verifyNoLine() << std::endl;
    std::cout << "VerifyNoLineIter: " << CH.verifyNoLineIter() << std::endl;
    CH.findLine(new_line);

    CH = CHTree<K,1>();
    data = {1, 3, 5, 6, 7, 9, 11, 12, 14, 15, 16, 18, 19, 20, 22};
    for(int i=0; i<data.size(); ++i) {
        RH.insert(data[i]);
        CH.insert({i,data[i]});
    }
    std::cout << "Must search for line: " << CH.findLine(new_line) << std::endl;
    std::cout << "Passed verification: " << CH.verifyLine(new_line) << std::endl;
    std::cout << "VerifyNoLine: " << CH.verifyNoLine() << std::endl;
    std::cout << "VerifyNoLineIter: " << CH.verifyNoLineIter() << std::endl;
    CH.findLine(new_line);
    if(!CH.verifyLine(new_line)) printListOfPoints(data);

    */

    std::cout << "Verification done: " << success << std::endl;

    return success;
}



int main(int argc, char* argv[]){
    if(argc > 1){
        return find_line(atoi(argv[1]));
    }

    return verificationTest<int,Quotient<long>>(5,true);
    //return find_line(1000);
}


