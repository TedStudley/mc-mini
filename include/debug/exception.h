#pragma once

#define THROW_WITH_TRACE(ex) \
    BOOST_THROW_EXCEPTION( ex << trace_info(get_backtrace()) )

#include <string>

#include <boost/exception/all.hpp>

#include "debug/backtrace.h"

typedef boost::error_info<struct tag_errmsg, std::string> errmsg_info;
typedef boost::error_info<struct tag_trace, std::string> trace_info;

struct ExceptionBase : virtual std::exception, virtual boost::exception { };

struct RuntimeError : public ExceptionBase {
  const char *what() const noexcept { return "Runtime Error"; };
};


struct InvalidArgument : public RuntimeError {
  const char *what() const noexcept { return "Argument Error"; };
};

struct ParserException : public RuntimeError {
  const char *what() const noexcept { return "Parser Error"; };
};

struct KeyNotFoundException : public ParserException {
  const char *what() const noexcept { return "Key not found"; };
};

struct ChildNotFoundException : public ParserException {
  const char *what() const noexcept { return "Child not found"; };
};
typedef boost::error_info<struct tag_missing_key, std::string> missing_key_info;
