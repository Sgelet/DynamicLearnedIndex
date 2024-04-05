#include <vector>
#include <random>
#include <chrono>
#include <cstring>
#include <cmath>
#include "ConvexHull/CHTree.h"
#include "ConvexHull/CQTree.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/convex_hull_2.h>
#include <CGAL/ch_graham_andrew.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2;

bool verificationTest(int verify_step, bool shuffle);

const double PI = std::acos(-1);

std::vector<std::pair<double,double>> generate_data(const size_t n, const int mode, const int seed) {
    std::vector<std::pair<double,double>> data(n);
    std::mt19937 engine(seed); // 42 for generation, 9001 for queries

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
void runtimeTest(const int test, const int window_size){
    // Read data
    std::mt19937 engine(42);
    double x;
    double y;
    std::vector<Point_2> data_p2;
    while(std::cin >> x){
        std::cin >> y;
        data_p2.emplace_back(x,y);
    }

    std::vector<std::pair<int,int>> updates(1000);
    for(int i=0; i<500; i++){
        updates.emplace_back(1,i);
        updates.emplace_back(0,i);
    }
    auto queries = generate_data(2<<20,1, 9001);

    std::shuffle(data_p2.begin(),data_p2.end(),engine);

    auto CQ = CQTree<K>();
    auto CH = CHTree<K>();

    std::vector<Point_2> out = {};

    std::cout << "Inserting"<<std::endl;

    auto t0 = hrc::now();
    auto t1 = hrc::now();
    auto acc = t1-t0;

    volatile bool b;
    for(size_t i=0; i<data_p2.size()/window_size; i++){
        t0 = hrc::now();

        if(test == 3) CGAL::ch_graham_andrew(data_p2.begin(),data_p2.begin()+(i+1)*window_size,std::back_inserter(out));
        else if(test == 4) CGAL::convex_hull_2(data_p2.begin(),data_p2.begin()+(i+1)*window_size,std::back_inserter(out));
        else {
            for (size_t j = i * window_size; j < (i + 1) * window_size; j++) {
                if(test == 1) CH.insert(data_p2[j]);
                else CQ.insert(data_p2[j]);
            }
        }

        t1 = hrc::now();
        acc += t1 - t0;
        std::cout << (i+1)*window_size << " " << (t1 - t0).count() * 1e-9 << " " << acc.count() * 1e-9;

        std::shuffle(queries.begin(),queries.end(),engine);

        t0 = hrc::now();

        for(auto q :queries) {
            if (test == 1) b ^= CH.covers({q.first, q.second});
            else if (test == 2) b ^= CQ.covers({q.first, q.second});
        }

        t1 = hrc::now();
        std::cout << " " << (t1-t0).count() * 1e-9;

        t0 = hrc::now();

        for(auto u: updates){
            if(test == 1){
                if(u.first) CH.remove(data_p2[u.second]);
                else CH.insert(data_p2[(i+1)*window_size+u.second]);
            }
            if(test == 2){
                if(u.first) CQ.remove(data_p2[u.second]);
                else CQ.insert(data_p2[(i+1)*window_size+u.second]);
            }
        }

        t1 = hrc::now();
        std::cout << " " << (t1-t0).count() * 1e-9;


        for(auto u: updates){
            if(test == 1){
                if(u.first) CH.insert(data_p2[u.second]);
                else CH.remove(data_p2[(i+1)*window_size+u.second]);
            }
            if(test == 2){
                if(u.first) CQ.insert(data_p2[u.second]);
                else CQ.remove(data_p2[(i+1)*window_size+u.second]);
            }
        }
        std::cout<<std::endl;

        out.clear();
    }

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
        std::cout << (data.size()/window_size)*window_size - i * window_size << " " << (t1 - t0).count() * 1e-9 << " " << acc.count() * 1e-9 << std::endl;
    }
     */
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
        auto data = generate_data(1<<atoi(argv[2]),atoi(argv[3]),42);
        for(auto e: data){
            std::cout << e.first << " " << e.second << std::endl;
        }
    }else if(!std::strcmp(argv[1],"RUN")){
        runtimeTest(atoi(argv[2]),atoi(argv[3]));
    }
}


bool verify(CHTree<K>& CH, std::vector<std::pair<double,double>>& data, int size){
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
        if(res[i].x() != out[i].x() || res[i].y() != out[i].y()){
            return false;
        }
    }
    return true;
}


bool verificationTest(int verify_step, bool shuffle) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::vector<std::pair<double,double>> data;
    auto CH = CHTree<K>();
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
        CH.insert({iter->first,iter->second});
        if(++acc % verify_step == 0) if(!verify(CH,data,acc)) return false;
    }

    // Now remove things
    if(shuffle) std::shuffle(data.begin(),data.end(),g);
    for(auto riter = data.rbegin(); riter != data.rend(); ++riter){
        CH.remove({riter->first,riter->second});
        if(--acc % verify_step == 0) if(!verify(CH,data,acc)) return false;
    }

    return true;

}

