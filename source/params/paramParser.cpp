#include <fstream>

#include "params/paramParser.h"

void ParamParser::load(const std::string filename) {
  std::ifstream paramFile(filename);

  if (paramFile.fail()) {
    THROW_WITH_TRACE(RuntimeError() <<
            errmsg_info("Failed to open parameter file '" + filename + "'."));
  }

  parse(paramFile);
  paramFile.close();
}

void ParamParser::parse(std::string &params) {
  std::stringstream paramStream(params);

  parse(paramStream);
}

void ParamParser::parse(std::istream &paramStream) {
  std::string lineBuf;
  std::vector<std::string> parseBuf, paramBuf;

  _result = new ParamTree;

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
        _result->addParam(paramBuf[1], parseBuf[1]);
      } else if (paramBuf[0] == "enter") {
        _result->addNode(paramBuf[1]);
        _result->moveUp(paramBuf[1]);
      } else if (paramBuf[0] == "leave") {
        _result->moveDown();
      } else if (paramBuf[0][0] == '#') {
        // Do nothing
      } else {
        std::cerr << "Unrecognized parameter type: " << lineBuf << std::endl;
      }
    }

    parseBuf.clear();
    paramBuf.clear();

    getline(paramStream, lineBuf);
  }

  std::cerr << "Parameter tree built!" << std::endl << std::endl;
}
