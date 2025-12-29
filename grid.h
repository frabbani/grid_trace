#pragma once

#include "geom.h"

#define GRID_MAX 1000000
#define GRID_DIM (2 * GRID_MAX + 1)
#define GRID_KEY(v, k)                                                         \
  do {                                                                         \
    int32 x = CLAMP(v.x, -GRID_MAX, +GRID_MAX);                                \
    int32 y = CLAMP(v.y, -GRID_MAX, +GRID_MAX);                                \
    int32 z = CLAMP(v.z, -GRID_MAX, +GRID_MAX);                                \
    uint64 ux = (uint64)(x + GRID_MAX);                                        \
    uint64 uy = (uint64)(y + GRID_MAX);                                        \
    uint64 uz = (uint64)(z + GRID_MAX);                                        \
    k = ux + uy * GRID_DIM + uz * GRID_DIM * GRID_DIM;                         \
  } while (0);

struct NS_grid_cell_s {
  struct NS_aabb_s aabb;
  struct ivec3_s coord;
  uint64 key;
};