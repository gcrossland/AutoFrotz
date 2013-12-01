#ifndef HEADER_ALREADYINCLUDED
#define HEADER_ALREADYINCLUDED

#include <vector>
#include <string>
#include <tuple>
#include "libraries/autofrotz.hpp"

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
int main (int argc, char *argv[]);
size_t readLine (char *b, size_t bSize);
void printOutput (const char *begin, const char *end);
void printOutput (const std::string &o);

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
#endif
