#ifndef PROFILER_HPP_
#define PROFILER_HPP_

#include <vector>

#include "global.hpp"

class Profiler {
 public:
  Profiler(std::vector<unsigned char>& buf, std::vector<struct tape_info>& ti)
      : _buf{buf}, _ti{ti} {}

  Profiler() = default;

  /// @brief Run profiling on BF buffer
  void RunProfile();

 private:
  std::vector<unsigned char> _buf;
  std::vector<struct tape_info> _ti;
  /// @brief Loop info for simple loops.
  std::vector<struct loop_info> _sl;
  /// @brief Loop info for non-simple loops.
  std::vector<struct loop_info> _nsl;

  /// @brief Print profiling info for the tape.
  void PrintTapeInfo();
  /// @brief Get loop info in the tape.
  void GetLoopInfo();
  /// @brief Sort loop info.
  void SortLoopInfo();
  /// @brief Print simple and non-simple loop info.
  void PrintLoopInfo();
};

#endif  // PROFILER_HPP_