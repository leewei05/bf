#include "util.hpp"

#include <iostream>
#include <string>

void Die(std::string msg) {
  std::cerr << msg << "\n";
  std::exit(1);
}

std::string LoopTyToString(int t) {
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

void PrintBuf(std::vector<unsigned char>& buf) {
  for (auto& b : buf) {
    std::cout << b;
  }

  std::cout << '\n';
}