#include "compiler.hpp"

#include <vector>
#include <map>
#include <stack>

#include "global.hpp"
#include "util.hpp"

/// @brief BF tape
static unsigned char tape[SIZE] = {0};

void Compiler::ComputeBranch() {
  std::stack<int> stk;
  std::map<int, int> m;
  for (int i = 0; i < _buf.size(); i++) {
    char c = _buf[i];
    if (c == '[') {
      stk.push(i);
    } else if (c == ']' && !stk.empty()) {
      int leftB = stk.top();
      m.insert({i, leftB});
      m.insert({leftB, i});
      stk.pop();
    }
  }

  _target = m;
}

std::vector<struct tape_info> Compiler::Interp(bool enable_profiling) {
  int pc = 50000;
  std::vector<struct tape_info> ti(SIZE);
  for (int i = 0; i < _buf.size(); i++) {
    switch (_buf[i]) {
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
        if (!enable_profiling)
          putchar(tape[pc]);
        break;
      case ',':
        ti.at(i).count++;
        if (!enable_profiling)
          tape[pc] = getchar();
        break;
      case '[':
        ti.at(i).count++;
        if (!tape[pc]) {
          i = _target.at(i);
        }
        break;
      case ']':
        ti.at(i).count++;
        if (tape[pc]) {
          i = _target.at(i);
        }
        break;
      default:
        break;
    }
  }

  return ti;
}