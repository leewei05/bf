#include "profiler.hpp"

#include <iostream>
#include <string>
#include <utility>
#include <map>

#include "global.hpp"
#include "util.hpp"

/// @brief BF character
static std::string bfc = "><+-[].,";

void Profiler::RunProfile() {
  PrintTapeInfo();
  GetLoopInfo();
  SortLoopInfo();
  PrintLoopInfo();
}

std::map<int, struct loop_info> Profiler::RunProfileGetLoopInfo() {
  GetLoopInfo();

  std::map<int, struct loop_info> m;
  for (auto& sli : _sl) {
    m[sli.pos] = sli;
  }

  for (auto& nsli : _nsl) {
    m[nsli.pos] = nsli;
  }

  return m;
}

void Profiler::PrintTapeInfo() {
  std::cout << "Instructions profile:\n";
  for (int i = 0; i < _buf.size(); i++) {
    if (bfc.find(_buf[i]) != std::string::npos) {
      std::cout << i << ": " << _buf.at(i) << "\n";
    }
  }
}

void Profiler::GetLoopInfo() {
  std::vector<struct loop_info> sl;
  std::vector<struct loop_info> nsl;
  // Is inside loop or not
  bool leftBrack = false;
  // Is simple loop or not
  bool isSimple = false;
  // Has no > < inside the loop
  bool noShift = true;
  // Has only > < inside the loop
  bool onlyShift = true;
  // Left Bracket position
  int leftPos = 0;
  // Shift position that starts from left bracket
  int shift = 0;
  // Value change in p[0]
  int change = 0;
  for (int i = 0; i < _buf.size(); i++) {
    switch (_buf.at(i)) {
      case '[': {
        leftBrack = true;
        noShift = true;
        onlyShift = true;
        isSimple = true;
        leftPos = i;
        change = 0;
        shift = 0;
      } break;
      case ']': {
        if (leftBrack) {
          // int body = _ti[leftPos + 1].count;
          struct loop_info li = {.pos = leftPos, .count = 0};
          if (isSimple && shift == 0 && ((change == 1) || (change == -1))) {
            li.type = (noShift ? loop_info::loop_type::NoShift
                               : loop_info::loop_type::Shift);
            sl.push_back(li);
          } else {
            li.type = (onlyShift ? loop_info::loop_type::Scan
                                 : loop_info::loop_type::Other);
            if (!isSimple) {
              li.type = loop_info::loop_type::IO;
            }

            nsl.push_back(li);
          }
          change = 0;
          shift = 0;
          leftBrack = false;
        }
      } break;
      case '>': {
        noShift = false;
        shift++;
      } break;
      case '<': {
        noShift = false;
        shift--;
      } break;
      case '+': {
        onlyShift = false;
        if (shift == 0)
          change++;  // p[0]++
      } break;
      case '-': {
        onlyShift = false;
        if (shift == 0)
          change--;  // p[0]--
      } break;
      case '.': {
        isSimple = false;
      } break;
      case ',': {
        isSimple = false;
      } break;
      default:
        break;
    }
  }

  _sl = std::move(sl);
  _nsl = std::move(nsl);
}

namespace {
void SortLoop(std::vector<struct loop_info>& v) {
  std::stable_sort(v.begin(), v.end(),
                   [](const struct loop_info& l1, const struct loop_info& l2) {
                     return l1.count > l2.count;
                   });
}

void PrintLoop(std::vector<unsigned char>& buf,
               std::vector<struct loop_info>& lis) {
  for (auto& li : lis) {
    std::cout << li.pos << ": ";
    int j = li.pos;
    while (buf[j] != ']') {
      if (bfc.find(buf[j]) != std::string::npos)
        std::cout << buf[j];

      j++;
    }
    std::cout << buf[j];  // print ]
    std::cout << ": " << LoopTyToString(li.type) << "\n";
  }
}
}  // namespace

void Profiler::SortLoopInfo() {
  SortLoop(_sl);
  SortLoop(_nsl);
}

void Profiler::PrintLoopInfo() {
  std::cout << "\nSimple loops: \n";
  PrintLoop(_buf, _sl);
  std::cout << "\nNon-simple loops: \n";
  PrintLoop(_buf, _nsl);
}
