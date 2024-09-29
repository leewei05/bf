#ifndef PROFILER_HPP_
#define PROFILER_HPP_

#include <vector>
#include <map>

#include "global.hpp"

/// @brief This struct stores loop info in a BF program.
struct loop_info {
  int pos;  // position in the buffer
  enum loop_type {
    NoShift = 0,  // Simple no shift [-] or [+]
    Shift,        // Simple with shifts [-<+>]
    Scan,         // Non Simple Scan [>>>>] or [<<<<]
    Other,        // Other Non Simple
    IO,           // With i/o
  } type;
  int count;  // count of the loop body
};

class Profiler {
 public:
  Profiler(std::vector<unsigned char>& buf)
      : _buf{buf} {}

  Profiler() = default;

  /// @brief Run profiling on BF buffer.
  void RunProfile();
  /// @brief Run profiling and return loop info map.
  std::map<int, struct loop_info> RunProfileGetLoopInfo();

 private:
  std::vector<unsigned char> _buf;
  // std::vector<struct tape_info> _ti;
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