#include <iostream>
#include <string>
#include <map>

#include "debug/exception.h"
#include "parser/paramTree.h"

using namespace std;

ParamNode::~ParamNode () {
  for (map<string, ParamNode *>::iterator it = children.begin(); it != children.end(); ++it) {
    delete (it->second);
  }
}

ParamTree::ParamTree () {
  rootNode = focusNode = new ParamNode;
  focusNode->parent = NULL;
  cerr << "Initialized parameter tree." << endl;
}

ParamTree::~ParamTree () {
  delete rootNode;
}

void ParamTree::moveUp (string key) {
  if (focusNode->children.count (key)) {
    map<string, ParamNode *>::iterator iter = focusNode->children.find (key);

    focusNode = iter->second;
  } else {
    THROW_WITH_TRACE(KeyNotFoundException() <<
            errmsg_info("Child with key '" + key + "' not found"));
  }
}

void ParamTree::moveDown () {
  if (focusNode->parent == NULL) {
    THROW_WITH_TRACE(ParserException() <<
            errmsg_info("Popped off root of parameter tree"));
  } else {
    focusNode = focusNode->parent;
  }
}

string ParamTree::getParam (string key) {
  if (focusNode->params.count (key)) {
    return focusNode->params[key];
  } else {
    THROW_WITH_TRACE(KeyNotFoundException() <<
            errmsg_info("Param with key '" + key + "' not found"));
  }
}

void ParamTree::addNode (string key) {
  if (focusNode->children.count (key)) {
    THROW_WITH_TRACE(ParserException() <<
            errmsg_info("Child with key '" + key + "' already exists"));
  } else {
    ParamNode * newNode = new ParamNode;
    newNode->parent = focusNode;
    focusNode->children.insert(pair<string, ParamNode *> (key, newNode));
  }
}

void ParamTree::delNode (string key) {
  if (focusNode->children.count (key)) {
    ParamNode * match = focusNode->children.find (key)->second;
    focusNode->children.erase (key);
    delete match;
  } else {
    THROW_WITH_TRACE(KeyNotFoundException() <<
            errmsg_info("Child with key '" + key + "' not found"));
  }
}

void ParamTree::addParam (string key, string param) {
  focusNode->params.insert(pair<string, string> (key, param));
  cerr << "Wrote " << key << " as " << param << endl;
}

void ParamTree::delParam (string key) {
  if (focusNode->params.count (key)) {
    focusNode->params.erase (key);
  } else {
    THROW_WITH_TRACE(KeyNotFoundException() <<
            errmsg_info("Param with key '" + key + "' not found"));
  }
}
