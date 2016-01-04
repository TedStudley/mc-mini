#include <iostream>
#include <string>
#include <map>

#include "debug/exception.h"
#include "params/paramTree.h"

ParamNode::~ParamNode () {
  for (auto child_entry : children) {
    delete (child_entry.second);
  }
}

ParamTree::ParamTree () {
  rootNode = focusNode = new ParamNode;

  focusNode->parent = NULL;
  focusNode->sectionKey = "<root>";

  std::cerr << "Initialized parameter tree." << std::endl;
}

ParamTree::~ParamTree () {
  delete rootNode;
}

inline bool ParamTree::hasChild(std::string key) {
  return focusNode->children.count(key);
}

void ParamTree::moveUp(std::string key) {
  if (hasChild(key)) {
    focusNode = focusNode->children.find(key)->second;
  } else {
    THROW_WITH_TRACE(KeyNotFoundException() <<
            errmsg_info("Child with key not found.") <<
            missing_key_info(key));
  }
}

void ParamTree::moveDown () {
  ParamNode *parentNode = focusNode->parent;

  if (parentNode == NULL) {
    // Check to make sure we're not trying to pop from the root node.
    THROW_WITH_TRACE(ParserException() <<
            errmsg_info("Popped off root of parameter tree"));
  } else if (focusNode->isTemp) {
    // If the focus is a temporary node, delete it and remove it from the
    // parent's child map before moving down.
    focusNode->~ParamNode();
    parentNode->children.erase(focusNode->sectionKey);
  }

  focusNode = parentNode;
}

inline bool ParamTree::hasParam(std::string key) {
  return focusNode->params.count(key);
}

std::string ParamTree::getParam(std::string key) {
  if (hasParam(key)) {
    return focusNode->params[key];
  } else {
    THROW_WITH_TRACE(KeyNotFoundException() <<
            errmsg_info("Parameter with key not found.") <<
            missing_key_info(key));
  }
}

void ParamTree::addNode(std::string key, bool isTemp) {
  if (focusNode->children.count (key)) {
    THROW_WITH_TRACE(ParserException() <<
            errmsg_info("Child with key '" + key + "' already exists"));
  } else {
    ParamNode *newNode = new ParamNode;

    newNode->isTemp = isTemp;
    newNode->sectionKey = key;
    newNode->parent = focusNode;

    focusNode->children.insert(std::pair<std::string, ParamNode *>(key, newNode));
  }
}

void ParamTree::delNode(std::string key) {
  if (focusNode->children.count (key)) {
    ParamNode *match = focusNode->children.find(key)->second;
    focusNode->children.erase(key);

    delete match;
  } else {
    THROW_WITH_TRACE(KeyNotFoundException() <<
            errmsg_info("Child with key not found") <<
            missing_key_info(key));
  }
}

void ParamTree::addParam(std::string key, std::string param) {
  focusNode->params.insert(std::pair<std::string, std::string>(key, param));
  std::cerr << "Wrote " << key << " as " << param << std::endl;
}

void ParamTree::delParam(std::string key) {
  if (hasParam(key)) {
    focusNode->params.erase(key);
  } else {
    THROW_WITH_TRACE(KeyNotFoundException() <<
            errmsg_info("Param with key not found.") <<
            missing_key_info(key));
  }
}
