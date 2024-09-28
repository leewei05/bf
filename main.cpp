#include <algorithm>
#include <cstring>
#include <cxxopts.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "compiler.hpp"
#include "global.hpp"
#include "util.hpp"

/// @brief BF tape
static unsigned char tape[SIZE] = {0};

/// @brief BF character
static std::string bfc = "><+-[].,";

namespace {
/// @brief Match branch targets.
std::map<int, int> m{};

std::map<int, int> compute_branch(std::vector<unsigned char>& buf) {
  std::stack<int> stk;
  for (int i = 0; i < buf.size(); i++) {
    char c = buf[i];
    if (c == '[') {
      stk.push(i);
    } else if (c == ']' && !stk.empty()) {
      int leftB = stk.top();
      m.insert({i, leftB});
      m.insert({leftB, i});
      stk.pop();
    }
  }

  return m;
}

}  // namespace

std::vector<struct tape_info> interp(std::vector<unsigned char>& buf, bool p) {
  int pc = 50000;
  std::vector<struct tape_info> ti(SIZE);
  for (int i = 0; i < buf.size(); i++) {
    switch (buf[i]) {
      case '>':
        if (pc + 1 > SIZE) {
          die(">: out of bound!");
        }

        ti.at(i).count++;
        ++pc;
        break;
      case '<':
        if (pc - 1 < 0) {
          die("<: out of bound!");
        }

        ti.at(i).count++;
        --pc;
        break;
      case '+':
        ti.at(i).count++;
        ++tape[pc];
        break;
      case '-':
        ti.at(i).count++;
        --tape[pc];
        break;
      case '.':
        ti.at(i).count++;
        if (!p)
          putchar(tape[pc]);
        break;
      case ',':
        ti.at(i).count++;
        if (!p)
          tape[pc] = getchar();
        break;
      case '[':
        ti.at(i).count++;
        if (!tape[pc]) {
          i = m.at(i);
        }
        break;
      case ']':
        ti.at(i).count++;
        if (tape[pc]) {
          i = m.at(i);
        }
        break;
      default:
        break;
    }
  }

  return ti;
}

void sort_loop_info(std::vector<struct loop_info>& v) {
  std::stable_sort(v.begin(), v.end(),
                   [](const struct loop_info& l1, const struct loop_info& l2) {
                     return l1.count > l2.count;
                   });
}

void print_loops(std::vector<unsigned char>& buf,
                 std::vector<struct loop_info> lis) {
  for (auto& li : lis) {
    std::cout << li.pos << ": ";
    for (int j = li.pos; j <= m.at(li.pos); j++) {
      if (bfc.find(buf[j]) != std::string::npos)
        std::cout << buf[j];
    }
    std::cout << ": " << li.count << ": " << loop_type_to_string(li.type)
              << "\n";
  }
}

std::pair<std::vector<struct loop_info>, std::vector<struct loop_info>>
get_loop_info(std::vector<unsigned char>& buf,
              std::vector<struct tape_info>& ti) {
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
  for (int i = 0; i < buf.size(); i++) {
    char c = buf[i];
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
        int body = ti[leftPos + 1].count;
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

  return std::make_pair(sl, nsl);
}

void print_profile(std::vector<unsigned char>& buf,
                   std::vector<struct tape_info>& ti) {
  std::cout << "Instructions profile:\n";
  for (int i = 0; i < buf.size(); i++) {
    if (bfc.find(buf[i]) != std::string::npos) {
      std::cout << i << ": " << buf[i] << ": " << ti[i].count << "\n";
    }
  }
}

std::vector<struct tape_info> profile(std::vector<unsigned char>& buf) {
  return interp(buf, true);
}

std::vector<unsigned char> clean_buf(std::vector<unsigned char>& buf) {
  std::vector<unsigned char> new_buf;
  for (int i = 0; i < buf.size(); i++) {
    if (bfc.find(buf[i]) != std::string::npos) {
      new_buf.push_back(buf[i]);
    }
  }

  return new_buf;
}

std::vector<unsigned char> optimize(std::vector<unsigned char>& buf) {
  std::cout << "optimize!\n";
  // auto ti = profile(buf);
  print_buf(buf);
  /*
  while(p.sl.size() == 0) {
    p = profile(buf);
    buf = transform(buf, p);
  }
  */
  return buf;
}

void profile_helper(std::vector<unsigned char>& buf) {
  auto ti = profile(buf);
  print_profile(buf, ti);
  auto li = get_loop_info(buf, ti);
  auto sl = li.first;
  auto nsl = li.second;
  sort_loop_info(sl);
  std::cout << "\nSimple loops: \n";
  print_loops(buf, sl);

  sort_loop_info(nsl);
  std::cout << "\nNon-simple loops: \n";
  print_loops(buf, nsl);
}

int main(int argc, char** argv) {
  auto cmd_options = cxxopts::Options{argv[0], "A simple BF compiler."};

  cmd_options.custom_help("[options] file");
  cmd_options.add_options()("i, interp", "Interpret input program",
                            cxxopts::value<bool>()->default_value("true"))(
      "c, compile", "Compile input porgram into x86-64",
      cxxopts::value<bool>()->default_value("false"))(
      "p, profile", "Profile input porgram",
      cxxopts::value<bool>()->default_value("false"))(
      "O, optimize", "Turn on optimization",
      cxxopts::value<bool>()->default_value("false"))(
      "h, help", "Display available options");

  auto opts = cmd_options.parse(argc, argv);
  if (opts.count("help")) {
    std::cerr << cmd_options.help() << '\n';
    std::exit(0);
  }

  auto args = opts.unmatched();
  if (args.size() == 0) {
    std::cerr << "no input files" << '\n';
    std::exit(0);
  }

  // TODO: check argc
  std::ifstream file(args.at(0));
  std::istream_iterator<unsigned char> start(file), end;
  std::vector<unsigned char> fbuf(start, end);
  // remove whitespaces and comments
  std::vector<unsigned char> buf = clean_buf(fbuf);
  m = compute_branch(buf);

  auto compiler = Compiler(buf);

  if (opts["optimize"].as<bool>()) {
    buf = optimize(buf);
  }

  if (opts["profile"].as<bool>()) {
    profile_helper(buf);
  } else if (opts["compile"].as<bool>()) {
    compiler.Compile();
  } else if (opts["interp"].as<bool>()) {
    // interp(buf, false);
    compiler.Interp(false);
  }

  return 0;
}
