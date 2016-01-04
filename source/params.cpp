#include "params.h"
#include "params/paramTree.h"

Params::Params() :
    treeBase(new ParamTree) {}

Params::~Params() {
  delete treeBase;
}

void Params::push(std::string key) {
  treeBase->moveUp(key);
}

void Params::tryPush(std::string key) {
  if (!treeBase->hasChild(key)) {
    // If the child doesn't exist, create it with a 'temporary' flag
    treeBase->addNode(key, true);
  }
  push(key);
}

void Params::pop() {
  treeBase->moveDown();
}
