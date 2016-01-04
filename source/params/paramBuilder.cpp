#include "params/paramBuilder.h"

Params &ParamBuilder::getParams() {
  return *((Params *) this);
}
