#include <algorithm>
#include <cstring>
#include <cxxopts.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "compiler.hpp"
#include "global.hpp"
#include "util.hpp"

int main(int argc, char** argv) {
  auto cmd_options = cxxopts::Options{argv[0], "A simple BF compiler."};

  cmd_options.custom_help("[options] file");
  cmd_options.add_options()("i, interp", "Interpret input program",
                            cxxopts::value<bool>()->default_value("true"))(
      "c, compile", "Compile input porgram into x86-64",
      cxxopts::value<bool>()->default_value("false"))(
      "p, profile", "Profile input porgram",
      cxxopts::value<bool>()->default_value("false"))(
      "d, dump", "Dump BF buffer",
      cxxopts::value<bool>()->default_value("false"))(
      "v, voptimize", "Turn on vector optimization",
      cxxopts::value<bool>()->default_value("false"))(
      "l, loptimize", "Turn on loop optimization",
      cxxopts::value<bool>()->default_value("false"))(
      "O, optimize", "Turn on both vector and loop optimization",
      cxxopts::value<bool>()->default_value("false"))(
      "h, help", "Display available options");

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
  auto compiler = Compiler(fbuf);

  // optimize with both loop and vector
  if (opts["optimize"].as<bool>()) {
    compiler.Optimize(true);
    compiler.Compile(true);
    return 0;
  }
  if (opts["voptimize"].as<bool>()) {
    compiler.Compile(true);
    return 0;
  }
  if (opts["loptimize"].as<bool>()) {
    compiler.Optimize(true);
    compiler.Compile(false);
    return 0;
  }

  if (opts["dump"].as<bool>()) {
    compiler.Dump();
  } else if (opts["profile"].as<bool>()) {
    compiler.Profile();
  } else if (opts["compile"].as<bool>()) {
    compiler.Compile(false);
  } else if (opts["interp"].as<bool>()) {
    compiler.Interp(false);
  }

  return 0;
}
