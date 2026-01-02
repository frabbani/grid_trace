#pragma once

#include "vec.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern int g_tests_run;
extern int g_tests_failed;

#define EPS 1e-5f

#define ASSERT_EQ(a, b)                                                        \
  do {                                                                         \
    g_tests_run++;                                                             \
    if ((a) != (b)) {                                                          \
      g_tests_failed++;                                                        \
      printf("[FAIL] %s:%d: ASSERT_EQ(%s,%s) got %lld vs %lld\n", __FILE__,    \
             __LINE__, #a, #b, (long long)(a), (long long)(b));                \
      return;                                                                  \
    }                                                                          \
  } while (0)

static bool feq(float a, float b, float eps) {
  float da = fabsf(a - b);
  float scale = fmaxf(1.0f, fmaxf(fabsf(a), fabsf(b)));
  return da <= eps * scale;
}

static bool v3eq(struct vec3_s a, struct vec3_s b, float eps) {
  return feq(a.x, b.x, eps) && feq(a.y, b.y, eps) && feq(a.z, b.z, eps);
}

#define ASSERT_FEQ(a, b)                                                       \
  do {                                                                         \
    g_tests_run++;                                                             \
    float _aa = (a), _bb = (b);                                                \
    if (!feq(_aa, _bb, EPS)) {                                                 \
      g_tests_failed++;                                                        \
      printf("[FAIL] %s:%d: ASSERT_FEQ(%s,%s) got %.9g vs %.9g\n", __FILE__,   \
             __LINE__, #a, #b, (double)_aa, (double)_bb);                      \
    }                                                                          \
  } while (0)

#define ASSERT_V3EQ(a, b)                                                      \
  do {                                                                         \
    g_tests_run++;                                                             \
    struct vec3_s _aa = (a), _bb = (b);                                        \
    if (!v3eq(_aa, _bb, EPS)) {                                                \
      g_tests_failed++;                                                        \
      printf("[FAIL] %s:%d: ASSERT_V3EQ(%s,%s) got (%.6g %.6g %.6g) vs (%.6g " \
             "%.6g %.6g)\n",                                                   \
             __FILE__, __LINE__, #a, #b, (double)_aa.x, (double)_aa.y,         \
             (double)_aa.z, (double)_bb.x, (double)_bb.y, (double)_bb.z);      \
    }                                                                          \
  } while (0)

#define ASSERT_TRUE(expr)                                                      \
  do {                                                                         \
    g_tests_run++;                                                             \
    if (!(expr)) {                                                             \
      g_tests_failed++;                                                        \
      printf("[FAIL] %s:%d: ASSERT_TRUE(%s)\n", __FILE__, __LINE__, #expr);    \
    }                                                                          \
  } while (0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

#define ASSERT_EQ_U(a, b)                                                      \
  do {                                                                         \
    g_tests_run++;                                                             \
    uint _a = (uint)(a), _b = (uint)(b);                                       \
    if (_a != _b) {                                                            \
      g_tests_failed++;                                                        \
      printf("[FAIL] %s:%d: ASSERT_EQ_U(%s,%s) got %u vs %u\n", __FILE__,      \
             __LINE__, #a, #b, _a, _b);                                        \
      return;                                                                  \
    }                                                                          \
  } while (0)

#define ASSERT_EQ_I(a, b)                                                      \
  do {                                                                         \
    g_tests_run++;                                                             \
    int _a = (int)(a), _b = (int)(b);                                          \
    if (_a != _b) {                                                            \
      g_tests_failed++;                                                        \
      printf("[FAIL] %s:%d: ASSERT_EQ_I(%s,%s) got %d vs %d\n", __FILE__,      \
             __LINE__, #a, #b, _a, _b);                                        \
      return;                                                                  \
    }                                                                          \
  } while (0)
