#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <map>
#include "red_black_tree.h"
#include <optional>

int main(){
	rb_tree<int> tree;	
	
	std::vector s = { 2,3,3,4,5,21,0,213,3,3,4,5,4,6,2,5,435,25,25,28,28,24,423,2,3,4,5,213,123,123,123,4,3,2,3,1,3,2,1,2,3,1,-1,-2,-5,-3,111,110,102 };

	for (auto i : s) {
		tree.insert(i);
	}
	
	tree.erase(tree.begin());
	std::cout << std::endl;
	for(auto a:tree) {
		std::cout << a << ' ';
	}
		
	std::cout << '\n' << tree.height() << ' '<<s.size();
}