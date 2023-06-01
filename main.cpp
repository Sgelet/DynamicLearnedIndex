#include <vector>
#include <random>
#include <chrono>
#include <cstring>
#include <cmath>
#include "CQTree.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/convex_hull_2.h>
#include <CGAL/ch_graham_andrew.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2;

bool verificationTest(int verify_step, bool shuffle);

const double PI = std::acos(-1);

std::vector<std::pair<double,double>> generate_data(size_t n, int mode) {
    std::vector<std::pair<double,double>> data(n);
    //std::mt19937 engine(42); // For generation
    std::mt19937 engine(9001); // For queries

    if(mode == 0){
        double r = 2147483647;
        std::normal_distribution<double> d {0.,r/(2+std::log(n))};
        std::generate(data.begin(),data.end(),[&](){return std::make_pair(d(engine),d(engine));});
    } else if(mode == 1){
        double r = 1000;
        std::uniform_real_distribution<double> d {-r,r};
        std::generate(data.begin(),data.end(),[&](){return std::make_pair(d(engine),d(engine));});
    } else if(mode == 2){
        double r = 1000;
        std::uniform_real_distribution<double> d {-r,r};
        std::generate(data.begin(),data.end(),[&](){
            auto x = d(engine);
            auto y = d(engine);
            while(x*x + y*y > r*r){
                x = d(engine);
                y = d(engine);
            }
            return std::make_pair(x,y);
        });
    } else if(mode == 3){
        double r = 1000;
        double step = 2*PI/n;
        int acc = -1;
        std::generate(data.begin(),data.end(),[&](){
            acc++;
            return std::make_pair(r*sin(acc*step),r*cos(acc*step));
        });
    }

    return data;
}
using hrc = std::chrono::high_resolution_clock;
void runtimeTest(int window_size){
    // Read data
    std::mt19937 engine(42);
    double x;
    double y;
    std::vector<std::pair<double,double>> data;
    //std::vector<Point_2> data;
    while(std::cin >> x){
        std::cin >> y;
        data.emplace_back(x,y);
    }

    std::vector<std::pair<int,int>> queries(1000);
    for(int i=0; i<500; i++){
        queries.emplace_back(1,i);
        queries.emplace_back(0,i);
    }

    //auto queries = generate_data(2<<20,1);

    std::shuffle(data.begin(),data.end(),engine);

    auto CH = CQTree<double>();

    std::vector<Point_2> out = {};

    std::cout << "Inserting"<<std::endl;

    auto t0 = hrc::now();
    auto t1 = hrc::now();
    auto acc = t1-t0;

    bool b;
    for(size_t i=0; i<data.size()/window_size; i++){
        t0 = hrc::now();

        for(size_t j=i*window_size; j<(i+1)*window_size; j++){
            CH.insert(data[j].first,data[j].second);
        }

        //CGAL::ch_graham_andrew(data.begin(),data.begin()+(i+1)*window_size,std::back_inserter(out));
        t1 = hrc::now();
        acc += t1 - t0;
        std::cout << (i+1)*window_size << " " << (t1 - t0).count() * 1e-9 << " " << acc.count() * 1e-9;

        std::shuffle(queries.begin(),queries.end(),engine);

        t0 = hrc::now();

        for(auto q :queries){
            if(q.first) CH.remove(data[q.second].first,data[q.second].second);
            else CH.insert(data[(i+1)*window_size+q.second].first,data[(i+1)*window_size+q.second].second);
            //b = CH.covers(q.first,q.second);
        }

        t1 = hrc::now();
        std::cout << " " << (t1-t0).count() * 1e-9  <<std::endl;
        /*
        for(auto q :queries){
            if(q.first) CH.insert(data[q.second].first,data[q.second].second);
            else CH.remove(data[(i+1)*window_size+q.second].first,data[(i+1)*window_size+q.second].second);
        }
         */
        out.clear();
    }
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
        std::cout << (data.size()/window_size)*window_size - i * window_size << " " << (t1 - t0).count() * 1e-9 << " " << acc.count() * 1e-9 << std::endl;
    }
}

int main(int argc, char* argv[]){
    if(argc < 3){
        std::cout << "You must supply test-identifier and window size as arguments"<<std::endl;
        return -1;
    }
    if(!std::strcmp(argv[1],"VER")){
        std::cout << "Running verification against CGAL convex hull: ";
        if(verificationTest(atoi(argv[2]),false)) std::cout << "SUCCESS";
        else std::cout << "FAILURE";
        std::cout << std::endl;
    }else if(!std::strcmp(argv[1],"GEN")){
        auto data = generate_data(1<<atoi(argv[2]),atoi(argv[3]));
        for(auto e: data){
            std::cout << e.first << " " << e.second << std::endl;
        }
    }else if(!std::strcmp(argv[1],"RUN")){
        runtimeTest(atoi(argv[2]));
    }
}

// TODO:

bool verify(CQTree<double>& CH, std::vector<std::pair<double,double>>& data, int size){
    std::vector<Point_2> out = {};
    std::vector<Point_2> temp;
    temp.reserve(size);
    for(int i=0; i<size; i++){
        temp.emplace_back(data[i].first,data[i].second);
    }
    //CGAL::upper_hull_points_2(temp.begin(), temp.end(),std::back_inserter(out));
    CGAL::lower_hull_points_2(temp.begin(), temp.end(),std::back_inserter(out));
    auto res = CH.lowerHullPoints();
    for(int i=0; i<res.size(); i++){
        if(res[i].first != out[i].x() || res[i].second != out[i].y()){
            return false;
        }
    }
    return true;
}


bool verificationTest(int verify_step, bool shuffle) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::vector<std::pair<double,double>> data;
    auto CH = CQTree<double>();
    double x;
    double y;
    while(std::cin >> x){
        std::cin >> y;
        data.emplace_back(x,y);
    }
    if(shuffle) std::shuffle(data.begin(),data.end(),g);
    // Insert things
    int acc = 0;
    for(auto iter = data.begin(); iter != data.end(); ++iter) {
        CH.insert(iter->first,iter->second);
        if(++acc % verify_step == 0) if(!verify(CH,data,acc)) return false;
    }

    // Now remove things
    if(shuffle) std::shuffle(data.begin(),data.end(),g);
    for(auto riter = data.rbegin(); riter != data.rend(); ++riter){
        CH.remove(riter->first,riter->second);
        if(--acc % verify_step == 0) if(!verify(CH,data,acc)) return false;
    }

    return true;

}

