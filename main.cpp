#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>
#include <stack>
#include <cstring>
#include <map>

const int SIZE = 100000;
unsigned char tape[SIZE] = {0};

struct profile_info {
  unsigned char c;
  unsigned int count;
};
struct profile_info pi[SIZE] = {'0', 0};

void die(std::string msg) {
  std::cerr << msg << "\n";
  exit(1);
}

std::map<int, int> compute_branch(std::vector<unsigned char> buf) {
  std::map<int, int> m;
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


void interp(std::vector<unsigned char> buf, bool p) {
  std::map<int, int> m = compute_branch(buf);
  int pc = 50000;
  for (int i = 0; i < buf.size(); i++) {
    switch (buf[i]) {
      case '>':
        if (pc + 1 > SIZE) {
          die(">: out of bound!");
        }

        pi[i].count++;
        ++pc;
        break;
      case '<':
        if (pc - 1 < 0) {
          die("<: out of bound!");
        }

        pi[i].count++;
        --pc;
        break;
      case '+':
        pi[i].count++;
        ++tape[pc];
        break;
      case '-':
        pi[i].count++;
        --tape[pc];
        break;
      case '.':
        pi[i].count++;
        if (!p)
          putchar(tape[pc]);
        break;
      case ',':
        pi[i].count++;
        if (!p)
          tape[pc] = getchar();
        break;
      case '[':
        // start collecting info
        pi[i].count++;
        if (!tape[pc]) {
          i = m.at(i);
        }
        break;
      case ']':
        pi[i].count++;
        if (tape[pc]) {
          i = m.at(i);
        }
        break;
      default:
        break;
    }
  }
}

void profile(std::vector<unsigned char> buf) {
  std::string str = "><+-[].,";
  interp(buf, true);
  for (int i = 0; i < buf.size(); i++) {
    if (str.find(buf[i]) != std::string::npos) {
      std::cout << i << ": " << buf[i] << ": " << pi[i].count << "\n";
    }
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "require input file\n";
    return 0;
  }

  // TODO: check argc
  std::ifstream file(argv[argc - 1]);
  std::istream_iterator<unsigned char> start(file), end;
  std::vector<unsigned char> buf(start, end);

  if (!strcmp(argv[1], "-p")) {
    profile(buf);
  } else {
    interp(buf, false);
  }

  std::cout << "Normal Termination!\n";

  return 0;
}
