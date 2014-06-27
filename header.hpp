#ifndef HEADER_ALREADYINCLUDED
#define HEADER_ALREADYINCLUDED

#include <vector>
#include <string>
#include <tuple>
#include "libraries/autofrotz.hpp"

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
int main (int argc, char *argv[]);
size_t readLine (char8_t *b, size_t bSize);
int strcmp (const char8_t *o0, const char8_t *o1);
int atoi (const char8_t *o);
void printOutput (const char8_t *begin, const char8_t *end);
void printOutput (const core::u8string &o);

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
#endif
