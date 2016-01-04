#include <string>
#include <iostream>

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

#include "params/paramParser.h"

TEST(ParamParserTest, can_parse_example_params) {
  boost::filesystem::path params_path =
      boost::filesystem::system_complete("../../exampleParameters");

  ParamParser pp;
  ASSERT_NO_THROW(pp.load(params_path.string()));
}

TEST(ParamParserTest, should_ignore_comments) {
  std::stringstream mock_params;
  mock_params <<
      "enter realSection" << std::endl <<
      "  set x=42" << std::endl <<
      "leave" << std::endl <<
      "# enter fakeSection" << std::endl <<
      "set y=13" << std::endl <<
      "# leave";

  ParamParser parser;
  parser.parse(mock_params);
  Params &params = parser.getParams();

  // The 'realSection' section should exist
  EXPECT_NO_THROW(params.push("realSection"));
  // The parameter `x` should exist and have the correct value
  int value;
  EXPECT_NO_THROW(params.getParam<int>("x", value));
  EXPECT_EQ(value, 42);
  EXPECT_NO_THROW(params.pop());

  // The 'fakeSection' section should *not* exist.
  EXPECT_THROW(params.push("fakeSection"), KeyNotFoundException);
  // The parameter `y` should exist at the root level
  EXPECT_NO_THROW(params.getParam<int>("y", value));
  EXPECT_EQ(value, 13);
}
