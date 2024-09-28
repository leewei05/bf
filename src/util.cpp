#include "util.hpp"

#include <iostream>
#include <string>

void die(std::string msg) {
  std::cerr << msg << "\n";
  std::exit(1);
}

std::string loop_type_to_string(int t) {
  switch (t) {
    case 0:
      return "S(No shift)";
    case 1:
      return "S(Shift)";
    case 2:
      return "NS(Scan)";
    case 3:
      return "NS(Other)";
    case 4:
      return "NS(I/O)";
    default:
      return "N/A";
  }
}

void print_buf(std::vector<unsigned char>& buf) {
  for (auto& b : buf) {
    std::cout << b;
  }

  std::cout << '\n';
}