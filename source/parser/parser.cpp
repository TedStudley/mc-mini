#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "debug/exception.h"
#include "parser/paramTree.h"
#include "parser/parser.h"

using namespace std;

ParamParser::ParamParser (const string paramFile) {
  ifstream paramStream(paramFile);

  if (paramStream.fail()) {
    THROW_WITH_TRACE(RuntimeError() <<
            errmsg_info("Opening parameter file '" + paramFile + "' failed."));
  }
  string lineBuf;
  vector<string> parseBuf, paramBuf;

  this->treeBase = new ParamTree;

  getline (paramStream, lineBuf);

  while (!paramStream.eof()) {
    if (lineBuf != "") {
      if (lineBuf.find_first_of("=") == string::npos) {
        parseBuf.clear ();
        parseBuf.push_back (lineBuf);
      } else {
        parseBuf = stringSplit(lineBuf ,"=");
      }
      paramBuf = stringSplit(parseBuf[0]);

      if (paramBuf[0] == "set") {
        treeBase->addParam (paramBuf[1], parseBuf[1]);
      } else if (paramBuf[0] == "enter") {
        treeBase->addNode (paramBuf[1]);
        treeBase->moveUp (paramBuf[1]);
      } else if (paramBuf[0] == "leave") {
        treeBase->moveDown ();
      } else if (paramBuf[0][0] == '#') {
        // Do nothing
      } else {
        cerr << "Unrecognized parameter type: " << lineBuf << endl;
      }
    }

    parseBuf.clear (); paramBuf.clear ();
    getline (paramStream, lineBuf);
  }
  cerr << "Parameter tree built!" << endl << endl;
}

ParamParser::~ParamParser () {
  delete treeBase;
}

bool ParamParser::push (string key) {
  try {
    treeBase->moveUp (key);
    return true;
  } catch (exception& e) {
    throw std::runtime_error(string(e.what()) + ": " + key + "; Exiting now.");
  }
  return false;
}

bool ParamParser::tryPush (string key) {
  try {
    treeBase->moveUp (key);
    return true;
  } catch (exception& e) {
    cerr << e.what() << ": " << key << "; Continuing." << endl;
    return false;
  }
}

void ParamParser::pop () {
  try {
    treeBase->moveDown ();
  } catch (exception& e) {
    throw std::runtime_error(string(e.what()) + "; Exiting now.");
  }
}

void ParamParser::getParamString (string key, string& result) {
  try {
    result = treeBase->getParam (key);
  } catch (exception& e) {
    throw std::runtime_error(string(e.what()) + ": " + key + "; Exiting now.");
  }
  cerr << key << " = " << result << endl; 
}

void ParamParser::queryParamString (string key, string& result, const string defaultVal = "") {
  try {
    result = treeBase->getParam (key);
  } catch (exception& e) {
    cerr << e.what() << ": " << key << "; Continuing." <<  endl;
    result = defaultVal;
  }
  cerr << key << " = " << result << endl; 
}

void ParamParser::getParamStringVect (string key, vector<string>& result) {
  try {
    result = stringSplit (treeBase->getParam (key));
  } catch (exception& e) {
    throw std::runtime_error(string(e.what()) + ": " + key + "; Exiting now.");
  }
}

void ParamParser::queryParamStringVect (string key, vector<string>& result, const vector<string> defaultValue = vector<string>()) {
  try {
    result = stringSplit (treeBase->getParam (key));
  } catch (exception& e) {
    cerr << e.what() << ": " << key << "; Continuing." << endl;
    result = defaultValue;
  }
}

void ParamParser::getParamInt (string key, int& result) {
  try {
    string tempBuf = treeBase->getParam (key);

    result = stoi (tempBuf);
  } catch (exception& e) {
    throw std::runtime_error(string(e.what()) + ": " + key + "; Exiting now.");
  }
}

void ParamParser::queryParamInt (string key, int& result, const int defaultValue = 0) {
  try {
    string tempBuf = treeBase->getParam (key);

    result = stoi (tempBuf);
  } catch (exception& e) {
    cerr << e.what () << ": " << key << "; Continuing." << endl;
    result = defaultValue;
  }
}

void ParamParser::getParamIntVect (string key, vector<int>& result) {
  
  try {
    string tempBuf = treeBase->getParam (key);
    vector<string> tempVect = stringSplit (tempBuf);

    for (vector<string>::iterator it = tempVect.begin(); it != tempVect.end(); ++it) {
      result.push_back (stoi(*it));
    }
  } catch (exception& e) {
    throw std::runtime_error(string(e.what()) + ": " + key + "; Exiting Now.");
  }
}

void ParamParser::queryParamIntVect (string key, vector<int>& result, const vector<int> defaultValue = vector<int> ()) {
  try {
    string tempBuf = treeBase->getParam (key);
    vector<string> tempVect = stringSplit (tempBuf);

    for (vector<string>::iterator it = tempVect.begin(); it != tempVect.end(); ++it) result.push_back (stoi (*it));
  } catch (exception& e) {
    cerr << e.what () << ": " << key << "; Continuing." << endl;
    result = defaultValue;
  }
}

void ParamParser::getParamDouble (string key, double& result) {
  try {
    string tempBuf = treeBase->getParam (key);
    result = stod (tempBuf);
  } catch (exception& e) {
    throw std::runtime_error(string(e.what()) + ": " + key + "; Exiting now.");
  }
}

void ParamParser::queryParamDouble (string key, double& result, const double defaultValue = 0.0) {
  try {
    string tempBuf = treeBase->getParam (key);
    
    result = stod (tempBuf);
  } catch (exception& e) {
    cerr << e.what () << ": " << key << "; Continuing." << endl;
    result = defaultValue;
  }
}

void ParamParser::getParamDoubleVect (string key, vector<double>& result) {
  
  try {
    string tempBuf = treeBase->getParam (key);
    vector<string> tempVect = stringSplit (tempBuf);

    
    for (vector<string>::iterator it = tempVect.begin(); it != tempVect.end(); ++it) result.push_back (stod(*it));
  } catch (exception& e) {
    throw std::runtime_error(string(e.what ()) + ": " + key + "; Exiting Now.");
  }
}

void ParamParser::queryParamDoubleVect (string key, vector<double>& result, const vector<double> defaultValue = vector<double> ()) {
  try {
    string tempBuf = treeBase->getParam (key);
    vector<string> tempVect = stringSplit (tempBuf);

    for (vector<string>::iterator it = tempVect.begin(); it != tempVect.end(); ++it) result.push_back (stod (*it));
  } catch (exception& e) {
    cerr << e.what () << ": " << key << "; Continuing." << endl;
    result = defaultValue;
  }
}
