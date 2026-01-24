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
typedef void (*GridTr_dtor_func)(void *);
typedef void (*GridTr_move_func)(void *, void *);

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

#define SORT2(a, b)                                                            \
  do {                                                                         \
    if (a > b) {                                                               \
      SWAP(a, b);                                                              \
    }                                                                          \
  } while (0)

#define SORT3(a, b, c)                                                         \
  do {                                                                         \
    SORT2(a, b);                                                               \
    SORT2(a, c);                                                               \
    SORT2(b, c);                                                               \
  } while (0)

#define PI 3.14159265358979323846f
#define DEG2RAD(x) ((x) * PI / 180.0f)
#define RAD2DEG(x) ((x) * 180.0f / PI)

#define TOL 1e-6f
#define TOL_SQ 1e-12f

extern void *GridTr_allocmem(size_t size, const char *file, int line);
extern void GridTr_freemem(void *ptr);
extern void GridTr_prmemstats(void);

// clang-format off

#define PTR_SZ (sizeof(void *))
#define GridTr_new(size)  GridTr_allocmem(size, __FILE__, __LINE__)
#define GridTr_free(ptr) do{ if(ptr){GridTr_freemem(ptr); ptr = NULL; } } while(0)
#define GridTr_oftype(t) #t

#define MAYBE_RESIZE(data, n, max, sz)                                         \
  do {                                                                         \
    if (n >= max) {                                                            \
      max = MAX(max * 2, 4);                                                   \
      void *new_ptr = GridTr_new(max * sz);                                    \
      if (data) {                                                              \
        memcpy(new_ptr, data, n * sz);                                         \
        GridTr_free(data);                                                     \
      }                                                                        \
      data = new_ptr;                                                          \
    }                                                                          \
  } while (0)

#define MAYBE_RESIZE_FIX(data, n, max, sz, gr)                                 \
  do {                                                                         \
    if (n >= max) {                                                            \
      max = MAX(max + gr, 4);                                                  \
      void *new_ptr = GridTr_new(max * sz);                                    \
      if (data) {                                                              \
        memcpy(new_ptr, data, n * sz);                                         \
        GridTr_free(data);                                                     \
      }                                                                        \
      data = new_ptr;                                                          \
    }                                                                          \
  } while (0)

// clang-format on

// reentrant strtok
char *GridTr_strtok_r(char *str, const char *delims, char **saveptr);
