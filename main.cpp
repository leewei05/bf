#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>
#include <cstring>

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

void interp(std::vector<unsigned char> buf, bool p) {
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
        pi[i].count++;
        if (!tape[pc]) {
          int cond = 1;
          while (cond) {
            char curr = buf[++i];
            if (curr == ']') {
              cond--;
            } else if (curr == '[') { // match one more ]
              cond++;
            }
          }
        }
        break;
      case ']':
        pi[i].count++;
        if (tape[pc]) {
          int cond = 1;
          while (cond) {
            char curr = buf[--i];
            if (curr == '[') {
              cond--;
            } else if (curr == ']') { // match one more [
              cond++;
            }
          }
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
