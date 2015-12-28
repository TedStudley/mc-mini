#define BACKTRACE_STACK_SIZE 10
#include <sstream>

#include "debug/backtrace.h"

std::string get_backtrace() {
  void *buffer[BACKTRACE_STACK_SIZE];
  char **strings;
  size_t nptrs;

  std::ostringstream backtrace_stream;

  // get void*'s for all entries on the stack
  nptrs = backtrace(buffer, BACKTRACE_STACK_SIZE);
  backtrace_stream << "backtrace() returned " << nptrs << " addresses:" << std::endl;

  /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
   * would produce similar output to the following:
   */
  strings = backtrace_symbols(buffer, nptrs);
  if (strings == NULL) {
    perror("backtrace_symbols");
    exit(EXIT_FAILURE);
  }
  for (size_t idx = 1; idx < nptrs; ++idx) {
    backtrace_stream << strings[idx] << std::endl;
  }

  return backtrace_stream.str();
}

void handler(int sig) {
  std::cerr << "Error: signal " << sig << ":" << std::endl;
  std::cerr << get_backtrace();
  exit(EXIT_FAILURE);
}
