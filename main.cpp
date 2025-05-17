#include "include/Skiplist.h"
#include <iostream>
int main() {
  Skiplist skiplist;
  for(int i = 0; i < 20; ++i) {
    skiplist.Insert(std::to_string(i), std::to_string(i * 10));
  }
    for (auto it = skiplist.begin(); it != skiplist.end(); ++it) {
        auto value = it.getValue();
        std::cout << "Key: " << value.first << ", Value: " << value.second
                << std::endl;
    }
    std::cout << "Node count: " << skiplist.getnodecount() << std::endl;
  return 0;}