#include <gtest/gtest.h>

#include "params.h"

class ParamsTest : public testing::Test, protected Params {};

TEST_F(ParamsTest, cant_push_missing_section) {
  EXPECT_THROW(push("bogusSection"), KeyNotFoundException);
}

TEST_F(ParamsTest, can_try_push_missing_section) {
  EXPECT_NO_THROW(tryPush("bogusSection"));
  EXPECT_NO_THROW(pop());

  // Attempting to renter the same section should result in a failure
  EXPECT_THROW(
          push("bogusSection"),
          KeyNotFoundException);
}

TEST_F(ParamsTest, cant_get_missing_value) {
  std::string expectedValue;
  EXPECT_THROW(
          getParam<std::string>("bogusParam", expectedValue),
          KeyNotFoundException);
}

TEST_F(ParamsTest, can_query_missing_value) {
  std::string expectedValue;
  EXPECT_NO_THROW(queryParam<std::string>("bogusParam", expectedValue, "testValue"));

  EXPECT_EQ("testValue", expectedValue);
}

TEST_F(ParamsTest, can_get_string_params) {
  treeBase->addParam("testKey", "testValue");

  std::string expectedValue;
  getParam<std::string>("testKey", expectedValue);

  EXPECT_EQ("testValue", expectedValue);
}

TEST_F(ParamsTest, can_get_int_params) {
  treeBase->addParam("testKey", "42");

  int expectedValue;
  getParam<int>("testKey", expectedValue);

  EXPECT_EQ(42, expectedValue);
}

TEST_F(ParamsTest, can_get_float_params) {
  treeBase->addParam("testKey", "3.14");

  float expectedValue;
  getParam<float>("testKey", expectedValue);

  EXPECT_FLOAT_EQ(3.14, expectedValue);
}

TEST_F(ParamsTest, can_get_double_params) {
  treeBase->addParam("testKey", "3.14159");

  double expectedValue;
  getParam<double>("testKey", expectedValue);

  EXPECT_DOUBLE_EQ(3.14159, expectedValue);
}
