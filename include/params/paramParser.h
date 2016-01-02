#pragma once

#include <string>

#include "params/paramBuilder.h"

class ParamParser : public ParamBuilder {
public:
  void load(std::string paramFile);

  void parse(std::string &params);
  void parse(std::istream &paramStream);

protected:
  static std::vector<std::string> inline stringSplit(
          const std::string &source,
          const char *delimiter = " ",
          bool keepEmpty = false) {

    std::vector<std::string> results;

    size_t prev, next;
    prev = next = 0;

    while ((next = source.find_first_of(delimiter, prev)) != std::string::npos) {
      if (keepEmpty || (next - prev != 0)) {
        results.push_back(source.substr(prev, next - prev));
      }
      prev = next + 1;
    }

    if (prev < source.size()) {
      results.push_back(source.substr(prev));
    }
    return results;
  }
};
