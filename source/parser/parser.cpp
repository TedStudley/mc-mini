#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "debug/exception.h"
#include "parser/paramTree.h"
#include "parser/parser.h"

ParamParser::ParamParser(const std::string paramFile) {
  std::ifstream paramStream(paramFile);

  if (paramStream.fail()) {
    THROW_WITH_TRACE(RuntimeError() <<
            errmsg_info("Opening parameter file '" + paramFile + "' failed."));
  }
  std::string lineBuf;
  std::vector<std::string> parseBuf, paramBuf;

  this->treeBase = new ParamTree;

  getline(paramStream, lineBuf);

  while (!paramStream.eof()) {
    if (lineBuf != "") {
      if (lineBuf.find_first_of("=") == std::string::npos) {
        parseBuf.clear();
        parseBuf.push_back(lineBuf);
      } else {
        parseBuf = stringSplit(lineBuf, "=");
      }
      paramBuf = stringSplit(parseBuf[0]);

      if (paramBuf[0] == "set") {
        treeBase->addParam(paramBuf[1], parseBuf[1]);
      } else if (paramBuf[0] == "enter") {
        treeBase->addNode(paramBuf[1]);
        treeBase->moveUp(paramBuf[1]);
      } else if (paramBuf[0] == "leave") {
        treeBase->moveDown();
      } else if (paramBuf[0][0] == '#') {
        // Do nothing
      } else {
        std::cerr << "Unrecognized parameter type: '" << lineBuf << "'" << std::endl;
      }
    }

    parseBuf.clear(); paramBuf.clear();
    getline(paramStream, lineBuf);
  }
  std::cerr << "Parameter tree built!" << std::endl << std::endl;
}

ParamParser::~ParamParser() {
  delete treeBase;
}

bool ParamParser::push(std::string key) {
  treeBase->moveUp(key);
  return true;
}

bool ParamParser::tryPush(std::string key) {
  try {
    treeBase->moveUp(key);
    return true;
  } catch (KeyNotFoundException &e) {
    std::cerr << *boost::get_error_info<errmsg_info>(e) << "; continuing anyway." << std::endl;
    return false;
  }
}

void ParamParser::pop() {
  treeBase->moveDown();
}

void ParamParser::getParamString(std::string key, std::string &result) {
  result = treeBase->getParam(key);
  std::cerr << key << " = " << result << std::endl;
}

void ParamParser::queryParamString(std::string key, std::string &result, const std::string defaultVal = "") {
  try {
    getParamString(key, result);
  } catch (KeyNotFoundException &e) {
    std::cerr << *boost::get_error_info<errmsg_info>(e) << "; continuing with default." << std::endl;
    result = defaultVal;
    std::cerr << key << " = '" << result << "'" << std::endl;
  }
}

void ParamParser::getParamStringVect(std::string key, std::vector<std::string> &result) {
  std::string tempBuf = treeBase->getParam(key);
  std::vector<std::string> tempVect = stringSplit(tempBuf);

  result.clear();
  for (std::string elem : tempVect) {
    result.push_back(elem);
  }
}

void ParamParser::queryParamStringVect(
        std::string key,
        std::vector<std::string> &result,
        const std::vector<std::string> defaultValue = std::vector<std::string>()) {
  try {
    getParamStringVect(key, result);
  } catch (KeyNotFoundException &e) {
    std::cerr << *boost::get_error_info<errmsg_info>(e) << "; continuing with default." << std::endl;
    result = defaultValue;
  }
}

void ParamParser::getParamInt(std::string key, int &result) {
  std::string tempBuf = treeBase->getParam(key);
  result = stoi(tempBuf);
  std::cerr << key << " = " << result << std::endl;
}

void ParamParser::queryParamInt(std::string key, int &result, const int defaultValue = 0) {
  try {
    getParamInt(key, result);
  } catch (KeyNotFoundException &e) {
    std::cerr << *boost::get_error_info<errmsg_info>(e) << "; continuing with default." << std::endl;
    result = defaultValue;
    std::cerr << key << " = " << result << std::endl;
  }
}

void ParamParser::getParamIntVect(string key, std::vector<int> &result) {
  std::string tempBuf = treeBase->getParam(key);
  std::vector<std::string> tempVect = stringSplit(tempBuf);

  result.clear();
  for (std::string elem : tempVect) {
    result.push_back(stoi(elem));
  }
}

void ParamParser::queryParamIntVect(
        std::string key,
        std::vector<int> &result,
        const std::vector<int> defaultValue = std::vector<int>()) {
  try {
    getParamIntVect(key, result);
  } catch (KeyNotFoundException &e) {
    std::cerr << *boost::get_error_info<errmsg_info>(e) << "; continuing with default." << std::endl;
    result = defaultValue;
  }
}

void ParamParser::getParamDouble(std::string key, double &result) {
  string tempBuf = treeBase->getParam(key);
  result = stod(tempBuf);
  std::cerr << key << " = " << result << std::endl;
}

void ParamParser::queryParamDouble(std::string key, double &result, const double defaultValue = 0.0) {
  try {
    getParamDouble(key, result);
  } catch (KeyNotFoundException &e) {
    std::cerr << *boost::get_error_info<errmsg_info>(e) << "; continuing with default." << std::endl;
    result = defaultValue;
    std::cerr << key << " = " << result << std::endl;
  }
}

void ParamParser::getParamDoubleVect(std::string key, std::vector<double> &result) {
  std::string tempBuf = treeBase->getParam(key);
  std::vector<std::string> tempVect = stringSplit(tempBuf);

  for (std::string it : tempVect) {
    result.push_back(stod(it));
  }
}

void ParamParser::queryParamDoubleVect(
        std::string key,
        std::vector<double> &result,
        const std::vector<double> defaultValue = std::vector<double>()) {
  try {
    getParamDoubleVect(key, result);
  } catch (KeyNotFoundException &e) {
    cerr << *boost::get_error_info<errmsg_info>(e) << "; continuing with default." << std::endl;
    result = defaultValue;
  }
}
