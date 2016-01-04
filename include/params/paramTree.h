#pragma once

#include <string>
#include <map>

class ParamNode {
  public:
    ~ParamNode();

    bool isTemp;
    std::string sectionKey;
    ParamNode *parent;

    std::map<std::string, ParamNode *> children;
    std::map<std::string, std::string> params;
};

class ParamTree {
  public:
    ParamTree ();
    ~ParamTree ();

    bool hasChild(std::string key);
    void moveUp(std::string key);
    void moveDown();

    bool hasParam(std::string key);
    std::string getParam(std::string key);

    void addNode(std::string key, bool isTemp = false);
    void delNode(std::string key);

    void addParam(std::string key, std::string value);
    void delParam(std::string key);

    ParamNode *rootNode;
    ParamNode *focusNode;
};
