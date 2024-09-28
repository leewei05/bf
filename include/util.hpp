#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <iostream>
#include <map>
#include <string>

/// @brief Print error message and exit 1.
void die(std::string msg);

/// @return A string that matches the loop type.
std::string loop_type_to_string(int t);

/// @brief Print the current BF buffer.
void print_buf(std::vector<unsigned char>& buf);

#endif  // UTIL_HPP_