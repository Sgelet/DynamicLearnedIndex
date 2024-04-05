#include <vector>
#include <random>
#include <chrono>
#include <cstring>
#include <cmath>
#include "CHTree.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point;
typedef K::Line_2 Line;

int main(int argc, char* argv[]){
   auto CH = CHTree<K>();

   std::vector<Point> points = {{1,1},{2,2},{3,3},{4,4}};//,{5,4},{6,3},{7,5},{8,0}};
   for(auto p: points){
       CH.insert(p);
   }

   Line res = Line();
   std::cout << "Find linear line: " << CH.findLine(res,1) << std::endl;

   CH.insert({5,9});

   std::cout << "Should fail: " << CH.findLine(res,1) << std::endl;

   auto CH2 = CHTree<K>();

   points = {{1,1},{2,4},{3,5},{4,6},{5,7},{6,8},{7,10}};
   for(auto p: points){
       CH2.insert(p);
   }
   std::cout << "Find wonky line: " << CH2.findLine(res,1) << std::endl;

   std::cout << "Convenience break";
}

