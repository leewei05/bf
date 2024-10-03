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
  out << "    # vector scan\n";
  out << "    xorl %r15d, %r15d\n";            // 15d accumulates offsets always positive
  out << "    jmp  .vec" << i << "\n";
  out << ".znf" << i << ":\n";
  out << "    addl    $16, %r15d\n";         // add to r15
  if (c == '>') {
    out << "    addl    $16, %r14d\n";
  } else if (c == '<') {
    out << "    subl    $16, %r14d\n";
  }
  out << ".vec" << i << ":\n";
  out << "    leaq   _tape(%rip), %rax\n";    // get _tape addr
  out << "    movslq %r14d, %rcx\n";
  out << "    movzbl (%rax,%rcx), %edx\n";
  if (c == '<') {
    // this should be 15, changing it would cause EXC_I386_GPFLT
    out << "    subl    $16, %r14d\n";         // move left 16 offset and load data
  }
  out << "    vmovdqa .vs1(%rip), %xmm0\n";   // indices 15 - 0
  out << "    movslq %r14d, %rcx\n";          // get pc
  out << "    pxor %xmm1, %xmm1\n";           // clear input
  out << "    movdqa (%rax, %rcx), %xmm1\n";  // load data
  if (c == '<') {
    out << "    psrldq $1, %xmm1\n";                 // shift right by 1 byte to remove offset 16
    out << "    vpinsrb $15, %edx, %xmm1, %xmm1\n";  // store tape[pc] to e15
  }
  out << "    movdq2q %xmm0, %mm0\n";
  out << "    pxor %xmm2, %xmm2\n";           // xmm2 all zeros
  out << "    pcmpeqb %xmm1, %xmm2\n";        // cmp input, 0
  out << "    vpmovmskb %xmm2, %eax\n";
  out << "    tzcnt  %eax, %esi\n";           // esi stores the position
  // if zero is not found
  out << "    cmpb    $32, %sil\n";
  out << "    je .znf" << i << "\n";
  if (c == '>') {
    out << "    addl    %esi, %r14d\n";          // add offset to pc
    out << "    addl    %r15d, %r14d\n";         // add accumulate offset to pc
  } else if (c == '<') {
    // bug memory scan larger than 15 won't work
    out << "    movl    $15, %r13d\n";
    out << "    subl    %esi, %r13d\n";
    out << "    movl    %r13d, %esi\n";          // esi has the offset

    out << "    subl    %esi, %r14d\n";         // sub offset to pc
    out << "    subl    %r15d, %r14d\n";
  }
  // printf
  out << "    addq    %r15, %rsi\n";
  out << "    leaq   .L.str(%rip), %rdi\n";
  out << "    xorl   %eax, %eax\n";
  out << "    callq  _printf\n";
  return i + 2;
}
}  // namespace

void Compiler::CodeGen(std::ofstream& out, bool sopt) {
  ComputeBranch();
  //std::cout << "Codegen:"  <<"\n";
  //PrintBuf(_buf);
  for (int i = 0; i < _buf.size(); i++) {
    switch (_buf[i]) {
      case 'y': {
        struct ir info = _ir.at(i);
        int at = info.shift;
        int c = info.change;
        out << "    # optimize y\n";
        out << "    leaq   _tape(%rip), %rax\n";  // rax = tape addr
        out << "    movslq %r14d, %rcx\n";
        out << "    movb   (%rax,%rcx), %dl\n";  // dl = tape[0]
        if (info.right) {
          out << "    addl   $" << at << ", %r14d\n";
        } else {
          out << "    subl   $" << at << ", %r14d\n";
        }
        out << "    movslq %r14d, %r10\n";
        out << "    movb   (%rax,%r10), %bl\n";  // bl = tape[0 + c or 0 - c]
        for (int i = 0; i < c; i++) {
          if (info.add) {
            out << "    addb   %dl, %bl\n";
          } else {
            out << "    subb   %dl, %bl\n";
          }
        }
        out << "    movb   %bl, (%rax,%r10)\n";
        if (info.right) {
          out << "    subl   $" << at << ", %r14d\n";
        } else {
          out << "    addl   $" << at << ", %r14d\n";
        }
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
        assert(false); // should not come here
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

std::vector<struct ir> Compiler::ComputeIR(std::vector<unsigned char>& v, int l, int r) {
  std::vector<struct ir> infos;
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
      struct ir info = {
        .right = shift > 0,
        .shift = std::abs(shift),
        .add = change > 0,
        .change = std::abs(change),
      };

      if (shift == 0) {
        goto noGen;
      }

      v.push_back('y');
      info.index = v.size() - 1;
      infos.push_back(info);
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
  return infos;
}

void Compiler::Optimize() {
  std::cout << "optimize!\n";

  ComputeBranch();
  Profiler p(_buf);
  auto m = p.RunProfileGetLoopInfo();
  std::vector<unsigned char> new_buf;
  std::map<int, struct ir> ir;
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
      std::vector<struct ir> infos = ComputeIR(new_buf, i, rb);
      for (struct ir info : infos) {
        struct ir n = {
          .index = info.index,
          .right = info.right,
          .shift = info.shift,
          .add = info.add,
          .change = info.change,
        };
        ir.insert({info.index, n});
      }

      i = rb + 1;
    } else {
      new_buf.push_back(_buf[i]);
      i++;
    }
  }

  _ir = ir;
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
  out << "    .p2align        4, 0x90\n";
  out << ".vs1:\n";
  out << "    .byte   15\n";
  out << "    .byte   14\n";
  out << "    .byte   13\n";
  out << "    .byte   12\n";
  out << "    .byte   11\n";
  out << "    .byte   10\n";
  out << "    .byte   9\n";
  out << "    .byte   8\n";
  out << "    .byte   7\n";
  out << "    .byte   6\n";
  out << "    .byte   5\n";
  out << "    .byte   4\n";
  out << "    .byte   3\n";
  out << "    .byte   2\n";
  out << "    .byte   1\n";
  out << "    .byte   0\n";
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
  out << ".L.str:\n";
  out << "    .asciz  \"pointer stops at %d\"\n";
}