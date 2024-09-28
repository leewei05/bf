#ifndef GLOBAL_HPP_
#define GLOBAL_HPP_

/// @brief This is the size of the tape.
const int SIZE = 100000;

/// @brief This struct stores every BF instruction's count.
struct tape_info {
  unsigned char c;
  unsigned int count;
};

#endif  // GLOBAL_HPP_