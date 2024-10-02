#include "compiler.hpp"

#include <cassert>
#include <fstream>
#include <map>
#include <stack>
#include <vector>

#include "global.hpp"
#include "profiler.hpp"
#include "util.hpp"

/// @brief BF character
static std::string bfc = "><+-[].,";

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

void Compiler::CleanBuf() {
  std::vector<unsigned char> new_buf;
  for (int i = 0; i < _buf.size(); i++) {
    if (bfc.find(_buf[i]) != std::string::npos) {
      new_buf.push_back(_buf[i]);
    }
  }

  _buf = std::move(new_buf);
}

namespace {
int VectorScan(std::ofstream& out, int i, unsigned char c) {
  // put a label
  // byte mask for input, 1, 2, 4
  // backword subtract offset first
  if (c == '>') {
    // set tape[pc]
    out << "    # scan [>]\n";
    out << "    jmp  .b" << i << "\n";
    out << ".znf" << i << ":\n";
    out << "    addl     $8, %r14d\n";
    out << ".b" << i << ":\n";
    out << "    leaq   _tape(%rip), %rax\n";
    out << "    movslq %r14d, %rcx\n";
    out << "    movzbl (%rax,%rcx), %edx\n";
    out << "    vmovd  %edx, %xmm0\n";  // store tape[pc] to e0
    for (int j = 1; j <= 7; j++) {
      // pc++
      out << "    addl   $1, %r14d\n";
      out << "    leaq   _tape(%rip), %rax\n";
      out << "    movslq %r14d, %rcx\n";
      out << "    movzbl (%rax,%rcx), %edx\n";
      out << "    vpinsrb $" << j
          << ", %edx, %xmm0, %xmm0\n";  // store tape[pc] to ej
    }
    // set xmm1 to all zeros
    out << "    pxor %xmm1, %xmm1\n";
    // cmp xmm1 and xmm0 each byte
    out << "    pcmpeqb %xmm1, %xmm0\n";  // results are stored in
                                          // xmm0
    out << "    pmovmskb %xmm0, %eax\n";
    out << "    tzcntl %eax, %eax\n";  // eax stores the position
                                       // relative to pc
    // if zero is not found
    out << "    cmpb    $8, %al\n";
    out << "    je .znf" << i << "\n";
    // add offset to pc
    out << "    addl    %eax, %r14d\n";
    // print zero location
    out << "    mov     %eax, %edi\n";
    out << "    addl   $48, %edi\n";
    out << "    callq  _putchar\n";
  } else if (c == '<') {
    out << "    # scan [<]\n";
    out << "    jmp  .b" << i << "\n";
    out << ".znf" << i << ":\n";
    out << "    subl     $8, %r14d\n";
    out << ".b" << i << ":\n";
    // set tape[pc]
    out << "    leaq   _tape(%rip), %rax\n";
    out << "    movslq %r14d, %rcx\n";
    out << "    movzbl (%rax,%rcx), %edx\n";
    out << "    vmovd  %edx, %xmm0\n";  // store tape[pc] to e0
    // for (int j = 7; j >= 1; j--) {
    for (int j = 1; j <= 7; j++) {
      // pc++
      out << "    subl   $1, %r14d\n";
      out << "    leaq   _tape(%rip), %rax\n";
      out << "    movslq %r14d, %rcx\n";
      out << "    movzbl (%rax,%rcx), %edx\n";
      out << "    vpinsrb $" << j
          << ", %edx, %xmm0, %xmm0\n";  // store tape[pc] to ej
    }
    // set xmm1 to all zeros
    out << "    pxor %xmm1, %xmm1\n";
    // cmp xmm1 and xmm0 each byte
    out << "    pcmpeqb %xmm1, %xmm0\n";  // results are stored in
                                          // xmm0
    out << "    pmovmskb %xmm0, %eax\n";
    out << "    tzcntl %eax, %eax\n";  // eax stores the position
                                       // relative to pc
    // if zero is not found
    out << "    cmpb    $8, %al\n";
    out << "    je .znf" << i << "\n";
    // sub offset to pc
    out << "    subl    %eax, %r14d\n";
    // print zero location
    out << "    mov     %eax, %edi\n";
    out << "    addl   $48, %edi\n";
    out << "    callq  _putchar\n";
  }
  return i + 3;
}
}  // namespace

void Compiler::CodeGen(std::ofstream& out, bool sopt) {
  ComputeBranch();
  //std::cout << "Codegen:" <<  _buf.size() <<"\n";
  //PrintBuf(_buf);
  for (int i = 0; i < _buf.size(); i++) {
    switch (_buf[i]) {
      case 'r': {
        // r1a7r2a:r3a3r4a1z
        int at = _buf[i + 1] - '0';
        unsigned char a = _buf[i + 2];
        int c = _buf[i + 3] - '0';
        out << "    # optimize r\n";
        out << "    leaq   _tape(%rip), %rax\n";  // rax = tape addr
        out << "    movslq %r14d, %rcx\n";
        out << "    movb   (%rax,%rcx), %dl\n";  // dl = tape[0]
        out << "    addl   $" << at << ", %r14d\n";
        out << "    movslq %r14d, %r10\n";
        out << "    movb   (%rax,%r10), %bl\n";  // bl = tape[0 + c or 0 - c]
        for (int i = 0; i < c; i++) {
          if (a == 'a') {
            out << "    addb   %dl, %bl\n";
          } else if (a == 's') {
            out << "    subb   %dl, %bl\n";
          }
        }
        out << "    movb   %bl, (%rax,%r10)\n";
        out << "    subl   $" << at << ", %r14d\n";
        i = i + 3;
      } break;
      case 'l': {
        int at = _buf[i + 1] - '0';
        unsigned char a = _buf[i + 2];
        int c = _buf[i + 3] - '0';
        out << "    # optimize r\n";
        out << "    leaq   _tape(%rip), %rax\n";  // rax = tape addr
        out << "    movslq %r14d, %rcx\n";
        out << "    movb   (%rax,%rcx), %dl\n";  // dl = tape[0]
        out << "    subl   $" << at << ", %r14d\n";
        out << "    movslq %r14d, %r10\n";
        out << "    movb   (%rax,%r10), %bl\n";  // bl = tape[0 + c or 0 - c]
        for (int i = 0; i < c; i++) {
          if (a == 'a') {
            out << "    addb   %dl, %bl\n";
          } else if (a == 's') {
            out << "    subb   %dl, %bl\n";
          }
        }
        out << "    movb   %bl, (%rax,%r10)\n";
        out << "    addl   $" << at << ", %r14d\n";
        i = i + 3;
      } break;
      case 'z':
        out << "    # optimize z\n";
        out << "    leaq   _tape(%rip), %rax\n";
        out << "    movslq %r14d, %rcx\n";
        out << "    movb   $0, %dl\n";
        out << "    movb   %dl, (%rax,%rcx)\n";
        break;
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
      case '[': {
        // scan [>] or [<]
        char c = _buf.at(i + 1);
        if (sopt && _buf.at(i + 2) == ']' && (c == '>' || c == '<')) {
          i = VectorScan(out, i, c);
        } else {
          out << ".b" << i << ":\n";
          out << "    leaq   _tape(%rip), %rax\n";
          out << "    movslq %r14d, %rcx\n";
          out << "    movb   (%rax,%rcx), %dl\n";
          out << "    cmpb   $0, %dl\n";
          out << "    je     "
              << ".b" << _target.at(i) << "\n";
        }
      } break;
      case ']':
        out << ".b" << i << ":\n";
        out << "    leaq   _tape(%rip), %rax\n";
        out << "    movslq %r14d, %rcx\n";
        out << "    movb   (%rax,%rcx), %dl\n";
        out << "    cmpb   $0, %dl\n";
        out << "    jne    "
            << ".b" << _target.at(i) << "\n";
        break;
      default: {
        std::cout << "default: " << _buf[i] << '\n';
      } break;
    }
  }
}

std::vector<struct tape_info> Compiler::Interp(bool enable_profiling) {
  int pc = 50000;
  unsigned char tape[SIZE] = {0};
  ComputeBranch();
  std::vector<struct tape_info> ti(SIZE);
  for (int i = 0; i < _buf.size(); i++) {
    switch (_buf[i]) {
      case '>':
        if (pc + 1 > SIZE) {
          Die(">: out of bound!");
        }

        ti.at(i).count++;
        ++pc;
        break;
      case '<':
        if (pc - 1 < 0) {
          Die("<: out of bound!");
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
        if (!enable_profiling) {
          putchar(tape[pc]);
        }
        break;
      case ',':
        ti.at(i).count++;
        if (!enable_profiling) {
          tape[pc] = getchar();
        }
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

void Compiler::Dump() {
  PrintBuf(_buf);
}

void Compiler::Profile() {
  ComputeBranch();
  Profiler p(_buf);
  p.RunProfile();
}

std::vector<unsigned char> Compiler::ComputeIR(int l, int r) {
  std::vector<unsigned char> v;
  // ignore [ the first - or +
  int i = l;
  int j = r;

  char currShift = '0';
  int shift = 0;
  int change = 0;
  bool endShift = false;
  i++;
  while (i <= j) {
    char c = _buf[i];
    if (change == 0) {
      if (c == '>') {
        shift++;
      } else if (c == '<') {
        shift--;
      }
    }

    if (change != 0 && (c == '>' || c == '<') && endShift) {
      if (shift == 0) {
        goto noGen;
      }
      if (shift < 0) {
        v.push_back('l');
      } else {
        v.push_back('r');
      }
      v.push_back(std::abs(shift) + '0'); // 123
      // std::cout << "shift amount:" << std::abs(shift) << "\n";
      if (change < 0) {
        v.push_back('s');
      } else {
        v.push_back('a');
      }
      v.push_back(std::abs(change) + '0');
    noGen:
      currShift = c;
      change = 0;
      endShift = false;
      if (c == '>') {
        shift++;
      } else if (c == '<') {
        shift--;
      }
    }

    if (c == '+') {
      change++;
      endShift = true;
    } else if (c == '-') {
      change--;
      endShift = true;
    }
    i++;
  }

  v.push_back('z');

  //for (int k = l; k <= r; ++k) {
  //  std::cout << _buf[k];
  //}
  //std::cout << '\n';
  // PrintBuf(v);
  return v;
}

void Compiler::Optimize() {
  std::cout << "optimize!\n";

  ComputeBranch();
  Profiler p(_buf);
  auto m = p.RunProfileGetLoopInfo();
  std::vector<unsigned char> new_buf;
  int i = 0;
  while (i < _buf.size()) {
    if (_buf[i] == '[' && _buf[i + 2] == ']' &&
        (_buf[i + 1] == '+' || _buf[i + 1] == '-')) {
      new_buf.push_back('z');
      i += 3;
    } else if (_buf[i] == '[' && m.count(i) != 0 &&
               m.at(i).type == loop_info::Shift) {
      // Simple loop with shifts
      int rb = _target.at(i);
      std::vector<unsigned char> v = ComputeIR(i, rb);
      // PrintBuf(v);
      new_buf.insert(new_buf.end(), v.begin(), v.end());
      i = rb + 1;
      // new_buf.push_back(_buf[i]);
    } else {
      new_buf.push_back(_buf[i]);
      i++;
    }
  }

  //std::cout << "original:\n";
  //PrintBuf(_buf);
  //std::cout << "new:\n";
  //PrintBuf(new_buf);
  if (new_buf != _buf) {
    _buf = new_buf;
    ComputeBranch();
  }
}

void Compiler::Compile(bool sopt) {
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
  out << "    movl   $160000, %r14d\n";
  CodeGen(out, sopt);
  // return 0
  out << "    xor    %eax, %eax\n";
  out << "    addq   $16, %rsp\n";
  out << "    pop    %rbp\n";
  out << "    ret\n";
  out << ".zerofill __DATA,__common,_tape,320000,4\n";
}