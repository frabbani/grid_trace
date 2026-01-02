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

struct GridTr_grid_cell_s {
  struct ivec3_s lrc; // z := layer, y := row, x := column
  uint64 hash;
  uint32 num_colliders;
  uint32 *colliders;
  struct GridTr_aabb_s aabb;
};

struct GridTr_grid_s {
  struct GridTr_hash_table_s *grid_table;
  struct GridTr_array_s *colliders;
  uint32 cell_size;
};