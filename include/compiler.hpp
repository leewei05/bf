#ifndef COMPILER_HPP_
#define COMPILER_HPP_

#include <map>
#include <utility>
#include <vector>

#include "profiler.hpp"

/// @brief Manages compilation, interpret and profiling
class Compiler {
 public:
  Compiler(std::vector<unsigned char> buf) : _buf{std::move(buf)} {
    CleanBuf();
    ComputeBranch();
  }

  /// @brief Interpret BF buffer.
  /// @note When enabling_profiling is true, it won't perform I/O operations.
  std::vector<struct tape_info> Interp(bool enable_profiling);

  /// @brief Dump BF buffer.
  void Dump();

  /// @brief Profile BF buffer and print profiling result
  void Profile();

  /// @brief Optimize BF buffer based on profiling info.
  void Optimize();

  // TODO: add output file
  /// @brief Compile BF into x86-64
  void Compile();

 private:
  /// @brief A buffer that stores the entire BF program.
  std::vector<unsigned char> _buf;
  /// @brief A map that matches branch target in BF.
  std::map<int, int> _target;

  /// @brief Compute branch target in BF buffer.
  /// @return A map that matches the target. If the key is the address of '[',
  /// then the address of ']' is the value.
  void ComputeBranch();

  /// @brief Remove comments and whitespaces in BF buffer.
  void CleanBuf();

  /// @brief Generate x86-64 from BF buffer.
  void CodeGen(std::ofstream& out);

  /// @brief Starting from index and return a vector of IR.
  std::vector<unsigned char> ComputeIR(int l, int r);
};

#endif  // COMPILER_HPP_