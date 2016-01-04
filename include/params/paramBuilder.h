#pragma once

#include "params.h"

/**
 * Intermediate 'interface' class for all param builder classes.
 *
 * All classes which inherit from ParamBuilder should have a way of
 * constructing a (temporarily hidden) set of parameters, which may be
 * accessed via the `getParams` method.
 */
class ParamBuilder : protected Params {
public:
  virtual Params &getParams();
};
