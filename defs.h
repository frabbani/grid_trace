#pragma once
#include <float.h>
#include <stdbool.h>
#include <stdint.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

// uint = native unsigned counter, not fixed-width
typedef unsigned int uint;

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(x, a, b) (MAX(a, MIN(x, b)))
#endif

#ifndef SQ
#define SQ(x) ((x) * (x))
#endif

#ifndef SWAP
#define SWAP(a, b)                                                             \
  do {                                                                         \
    __typeof__(a) t = a;                                                       \
    a = b;                                                                     \
    b = t;                                                                     \
  } while (0)
#endif

#define PI 3.14159265358979323846f
#define DEG2RAD(x) ((x) * PI / 180.0f)
#define RAD2DEG(x) ((x) * 180.0f / PI)

#define TOL 1e-6f
#define TOL_SQ 1e-12f
