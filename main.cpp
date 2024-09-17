#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>
#include <stack>
#include <cstring>
#include <map>
#include <utility>

const int SIZE = 100000;
unsigned char tape[SIZE] = {0};

struct profile_info {
  unsigned char c;
  unsigned int count;
};
struct profile_info pi[SIZE] = {'0', 0};
std::string bfc = "><+-[].,";
std::map<int, int> m;

void die(std::string msg) {
  std::cerr << msg << "\n";
  exit(1);
}

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


void interp(std::vector<unsigned char>& buf, bool p) {
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

void print_loops(std::vector<unsigned char>& buf, std::vector<std::pair<int, int>> v) {
  for (auto& [count, pos]: v) {
    std::cout << pos << ": ";
    for (int j = pos; j <= m.at(pos); j++) {
     if (bfc.find(buf[j]) != std::string::npos)
       std::cout << buf[j];
    }
    std::cout << ": " << count << "\n";
  }
}

void get_loops(std::vector<unsigned char>& buf) {
  bool leftB = false;
  bool is_simple = false;
  int leftPos = 0;
  std::string not_simple = "><.,";
  std::string simple = "+-";
  // simple loops
  std::vector<std::pair<int,int>> sl;
  // non simple loops
  std::vector<std::pair<int,int>> nsl;
  int count = 0;
  for (int i = 0; i < buf.size(); i++) {
    if (count > 1) {
      is_simple = false;
    }

    char c = buf[i];
    if (c == '[') {
      leftB = true;
      leftPos = i;
      is_simple = true;
      count = 0;
    } else if (leftB && c == ']') {
      int body = pi[leftPos + 1].count;
      if (is_simple) {
        sl.push_back(std::make_pair(body, leftPos));
      } else {
        nsl.push_back(std::make_pair(body, leftPos));
      }
      count = 0;
      leftB = false;
    } else if (leftB && not_simple.find(c) != std::string::npos) {
      is_simple = false;
    } else if (leftB && simple.find(c) != std::string::npos) {
      count++;
    }
  }

  std::cout << "\nSimple loops: \n";
  print_loops(buf, sl);

  std::cout << "\nNon-simple loops: \n";
  print_loops(buf, nsl);
}

void profile(std::vector<unsigned char>& buf) {
  interp(buf, true);
  std::cout << "Instructions profile:\n";
  for (int i = 0; i < buf.size(); i++) {
    if (bfc.find(buf[i]) != std::string::npos) {
      std::cout << i << ": " << buf[i] << ": " << pi[i].count << "\n";
    }
  }

  get_loops(buf);
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

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "require input file\n";
    return 0;
  }

  // TODO: check argc
  std::ifstream file(argv[argc - 1]);
  std::istream_iterator<unsigned char> start(file), end;
  std::vector<unsigned char> fbuf(start, end);
  // remove whitespaces and comments
  std::vector<unsigned char> buf = clean_buf(fbuf);
  m = compute_branch(buf);

  if (!strcmp(argv[1], "-p")) {
    profile(buf);
  } else {
    interp(buf, false);
  }

  std::cout << "Normal Termination!\n";

  return 0;
}
