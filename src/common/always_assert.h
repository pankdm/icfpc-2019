#pragma once

#include <cassert>
#include <cstdio>
#include <cstdlib>

#define ALWAYS_ASSERT(cond) \
  if (!(cond)) {            \
    abort();                \
  }

#define ALWAYS_ASSERTF(cond, ...)                                       \
  if (!(cond)) {                                                        \
    fprintf(stderr, "Assertion failed at %s:%d\n", __FILE__, __LINE__); \
    fprintf(stderr, __VA_ARGS__);                                       \
    fflush(stderr);                                                     \
    abort();                                                            \
  }
