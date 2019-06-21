#pragma once

#include <sstream>
#include <string>
#include <vector>

inline std::vector<std::string> Split(const std::string& s, char delimeter) {
  std::vector<std::string> vs;
  std::stringstream ss(s);
  for (std::string st; getline(ss, st, delimeter);) vs.emplace_back(st);
  return vs;
}
