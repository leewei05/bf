#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>

const int SIZE = 10000;
unsigned char tape[SIZE] = {0};

void die(std::string msg) {
  std::cerr << msg << "\n";
  exit(1);
}

void interp(std::vector<unsigned char> buf) {
  int pc = 0;
  for (int i = 0; i < buf.size(); i++) {
    switch (buf[i]) {
      case '>':
        if (pc + 1 > SIZE) die(">: out of bound!");
        ++pc;
        break;
      case '<':
        if (pc - 1 < 0) die("<: out of bound!");
        --pc;
        break;
      case '+':
        ++tape[pc];
        break;
      case '-':
        --tape[pc];
        break;
      case '.':
        putchar(tape[pc]);
        break;
      case ',':
        tape[pc] = getchar();
        break;
      case '[':
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

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "require input file\n";
  }

  std::ifstream file(argv[1]);
  std::istream_iterator<unsigned char> start(file), end;
  std::vector<unsigned char> buf(start, end);

  interp(buf);

  std::cout << "Normal Termination!\n";

  return 0;
}
