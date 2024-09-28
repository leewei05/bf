#ifndef GLOBAL_HPP_
#define GLOBAL_HPP_

/// @brief This is the size of the tape.
const int SIZE = 100000;

/// @brief This struct stores every BF instruction's count.
struct tape_info {
  unsigned char c;
  unsigned int count;
};

// TODO: move to profiler
/// @brief This struct stores loop info in a BF program.
struct loop_info {
  int pos;  // position in the buffer
  enum loop_type {
    NoShift = 0,  // Simple no shift [-] or [+]
    Shift,        // Simple with shifts [-<+>]
    Scan,         // Simple Scan [>>>>] or [<<<<]
    Other,        // Other Non Simple
    IO,           // With i/o
  } type;
  int count;  // count of the loop body
};

#endif  // GLOBAL_HPP_