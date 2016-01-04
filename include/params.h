#pragma once

#include <vector>
#include <string>

#include <boost/lexical_cast.hpp>

#include "debug/exception.h"
#include "params/paramTree.h"

/**
 * The Params object holds external values used to configure the simulation at
 * run-time.
 *
 * Different parameter sections are stored in a tree data structure, with each
 * section containing a map of parameter keys to values (in string
 * representation). Values are retrieved from a section using either the
 * `getParam` or the `queryParam` methods.
 */
class Params {
public:
  Params();
  Params(ParamTree *treeBase);

  ~Params();

  /**
   * Enter the child parameter section of the current parameter section under
   * the `key` entry in the child map.
   *
   * Fails with `ChildNotFoundException` if the specified section does not
   * exist.
   */
  void push(std::string key);

  /**
   * Attempt to enter the child parameter section of the current parameter
   * section under the `key` entry in the child map.
   *
   * If the specified section does not exist, a temporary node is created for
   * it so that default-value parameters can still be queried.
   */
  void tryPush(std::string key);

  /**
   * Return to the parent of the current parameter section.
   *
   * Fails with `RuntimeError` if the current section has no parent.
   */
  void pop();

  /**
   * Retrieves the parameter value under `key` and stores it in `result`.
   *
   * Fails with `KeyNotFoundException` if there is no parameter value under
   * `key`.
   */
  template <typename T>
  void getParam(
          std::string key,
          T &result) {
    result = boost::lexical_cast<T>(treeBase->getParam(key));
  }

  /**
   * Attempts to retrieve the parameter value under `key` and stores it in
   * `result`. If there is no such parameter value, the default value specified
   * by `default_value` is used instead.
   */
  template <typename T>
  void queryParam(
          std::string key,
          T &result,
          const T default_value) {
    try {
      // Attempt to get an existing value
      getParam(key, result);
    } catch (KeyNotFoundException &) {
      // No value could be found, so use the default value
      result = default_value;
    }
  }

protected:
  ParamTree *treeBase;
};
