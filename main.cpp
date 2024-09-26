#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>
#include <stack>
#include <cstring>
#include <map>
#include <utility>
#include <algorithm>

#include <cxxopts.hpp>

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

void sort_loop_info(std::vector<std::pair<int, int>>& v) {
  std::stable_sort(v.begin(), v.end(),
  [](const std::pair<int,int>& p1, const std::pair<int,int>& p2)
  { return p1.first > p2.first; });
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
  std::string not_simple = ",.";
  std::string simple = "+-";
  // simple loops
  std::vector<std::pair<int,int>> sl;
  // non simple loops
  std::vector<std::pair<int,int>> nsl;
  //int count = 0;
  int shift = 0;
  int change = 0;
  for (int i = 0; i < buf.size(); i++) {
    char c = buf[i];
    if (c == '[') {
      leftB = true;
      leftPos = i;
      is_simple = true;
      change = 0;
      shift = 0;
    } else if (leftB) {
      if (c == ']') {
        int body = pi[leftPos + 1].count;
        if (is_simple && shift == 0 && ((change == 1) || (change == -1))) {
          sl.push_back(std::make_pair(body, leftPos));
        } else {
          nsl.push_back(std::make_pair(body, leftPos));
        }
        change = 0;
        shift = 0;
        leftB = false;
      } else if (c == '>') {
        shift++;
      } else if (c == '<') {
        shift--;
      } else if (c == '+' && shift == 0) {
        change++;
      } else if (c == '-' && shift == 0) {
        change--;
      } else if (not_simple.find(c) != std::string::npos) {
        is_simple = false;
      }
    }
  }

  sort_loop_info(sl);
  std::cout << "\nSimple loops: \n";
  print_loops(buf, sl);

  sort_loop_info(nsl);
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

void codegen(std::ofstream& out, std::vector<unsigned char>& buf) {
  for (int i = 0; i < buf.size(); i++) {
    switch(buf[i]) {
      case '>':
        out << "    addl   $1, %r14d\n";
        break;
      case '<':
        out << "    subl   $1, %r14d\n";
        break;
      case '+':
        out << "    leaq   _tape(%rip), %rax\n";
        out << "    movslq %r14d, %rcx\n";
        out << "    movb   (%rax,%rcx), %dl\n";
        out << "    addb   $1, %dl\n";
        out << "    movb   %dl, (%rax,%rcx)\n";
        break;
      case '-':
        out << "    leaq   _tape(%rip), %rax\n";
        out << "    movslq %r14d, %rcx\n";
        out << "    movb   (%rax,%rcx), %dl\n";
        out << "    subb   $1, %dl\n";
        out << "    movb   %dl, (%rax,%rcx)\n";
        break;
      case '.':
        out << "    leaq   _tape(%rip), %rax\n";
        out << "    movslq %r14d, %rcx\n";
        out << "    movzbl (%rax,%rcx), %edi\n";
        out << "    callq  _putchar\n";
        break;
      case ',':
        out << "    callq  _getchar\n";
        out << "    movb   %al, %dl\n";
        out << "    leaq   _tape(%rip), %rax\n";
        out << "    movslq %r14d, %rcx\n";
        out << "    movb   %dl, (%rax,%rcx)\n";
        break;
      case '[':
        // check if loop is simple loop
        // if yes -> optimize
        // if no -> generate the following
        out << ".b" << i << ":\n";
        out << "    leaq   _tape(%rip), %rax\n";
        out << "    movslq %r14d, %rcx\n";
        out << "    movb   (%rax,%rcx), %dl\n";
        out << "    cmpb   $0, %dl\n";
        out << "    je     " << ".b" << m.at(i) << "\n";
        break;
      case ']':
        out << ".b" << i << ":\n";
        out << "    leaq   _tape(%rip), %rax\n";
        out << "    movslq %r14d, %rcx\n";
        out << "    movb   (%rax,%rcx), %dl\n";
        out << "    cmpb   $0, %dl\n";
        out << "    jne    " << ".b" << m.at(i) << "\n";
        break;
      default:
        break;
    }
  }
}

void compile(std::vector<unsigned char>& buf) {
  auto out = std::ofstream{"bf.s"};
  out << "    .section        __TEXT,__text,regular,pure_instructions\n";
  out << "    .build_version macos, 14, 0\n";
  out << "    .globl  _tape\n";
  out << "    .globl  _main\n";
  out << "    .p2align        4, 0x90\n\n";
  out << "_main:\n";
  out << "    push   %rbp\n";
  out << "    mov    %rsp, %rbp\n";
  out << "    subq   $16, %rsp\n";
  // int pc = 0; %r14d 32 bits
  out << "    movl   $50000, %r14d\n";
  codegen(out, buf);
  // return 0
  out << "    xor    %eax, %eax\n";
  out << "    addq   $16, %rsp\n";
  out << "    pop    %rbp\n";
  out << "    ret\n";
  out << ".zerofill __DATA,__common,_tape,100000,4\n";
}

int main(int argc, char** argv) {
  auto cmd_options = cxxopts::Options{
    argv[0],
    "A simple BF compiler."
  };

  cmd_options.custom_help("[options] file");
  cmd_options.add_options()
      ("i, interp", "Interpret input program", cxxopts::value<bool>()->default_value("true"))
      ("c, compile", "Compile input porgram into x86-64", cxxopts::value<bool>()->default_value("false"))
      ("p, profile", "Profile input porgram", cxxopts::value<bool>()->default_value("false"))
      ("h, help", "Display available options")
      ;

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

  if (opts["profile"].as<bool>()) {
    profile(buf);
  } else if (opts["compile"].as<bool>()) {
    compile(buf);
  } else if (opts["interp"].as<bool>()) {
    interp(buf, false);
  }

  return 0;
}
