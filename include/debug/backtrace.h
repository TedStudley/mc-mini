#include <iostream>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

std::string get_backtrace();

void handler(int sig);
