#include "profiler.hpp"

#include <iostream>
#include <string>
#include <utility>

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

void Profiler::PrintTapeInfo() {
  std::cout << "Instructions profile:\n";
  for (int i = 0; i < _buf.size(); i++) {
    if (bfc.find(_buf[i]) != std::string::npos) {
      std::cout << i << ": " << _buf.at(i) << ": " << _ti.at(i).count << "\n";
    }
  }
}

void Profiler::GetLoopInfo() {
  bool leftB = false;
  bool is_simple = false;
  int leftPos = 0;
  std::string not_simple = ",.";
  std::string simple = "+-";
  // simple loops
  std::vector<struct loop_info> sl;
  // non simple loops
  std::vector<struct loop_info> nsl;
  // int count = 0;
  int shift = 0;
  int change = 0;
  bool no_shift = true;
  bool only_shift = true;
  for (int i = 0; i < _buf.size(); i++) {
    char c = _buf[i];
    if (c == '[') {
      leftB = true;
      leftPos = i;
      is_simple = true;
      change = 0;
      shift = 0;
      no_shift = true;
      only_shift = true;
    } else if (leftB) {
      if (c == ']') {
        int body = _ti[leftPos + 1].count;
        if (is_simple && shift == 0 && ((change == 1) || (change == -1))) {
          struct loop_info li = {
              .pos = leftPos,
              .type = (no_shift ? loop_info::loop_type::NoShift
                                : loop_info::loop_type::Shift),
              .count = body,
          };
          sl.push_back(li);
        } else {
          struct loop_info li = {
              .pos = leftPos,
              .type = (only_shift ? loop_info::loop_type::Scan
                                  : loop_info::loop_type::Other),
              .count = body,
          };

          if (!is_simple) {
            li.type = loop_info::loop_type::IO;
          }

          nsl.push_back(li);
        }
        change = 0;
        shift = 0;
        leftB = false;
      } else if (c == '>') {
        no_shift = false;
        shift++;
      } else if (c == '<') {
        no_shift = false;
        shift--;
      } else if (c == '+' && shift == 0) {
        change++;
      } else if (c == '-' && shift == 0) {
        change--;
      } else if (c == '+') {
        only_shift = false;
      } else if (c == '-') {
        only_shift = false;
      } else if (not_simple.find(c) != std::string::npos) {
        is_simple = false;
      }
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
    std::cout << ": " << li.count << ": " << LoopTyToString(li.type) << "\n";
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
