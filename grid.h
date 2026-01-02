#pragma once

#include "collide.h"
#include <math.h>

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
  struct GridTr_hash_table_s *cell_table;
  struct GridTr_array_s *colliders;
  uint32 cell_size;
  struct GridTr_aabb_s aabb;
};

struct ivec3_s GridTr_get_grid_cell_p(struct vec3_s p, float cell_size) {
  struct ivec3_s lrc;
  float s = 1.0f / cell_size;
  lrc.x = (int32)floorf(p.x * s);
  lrc.y = (int32)floorf(p.y * s);
  lrc.z = (int32)floorf(p.z * s);
  return lrc;
}

void GridTr_get_collider_grid_cell_exts(
    const struct GridTr_collider_s *collider, float cell_size,
    struct ivec3_s *lrc_min, struct ivec3_s *lrc_max) {
  struct vec3_s min, max;
  // struct ivec3_s max = ivec3_set(INT32_MIN, INT32_MIN, INT32_MIN);
  // struct ivec3_s min = ivec3_set(INT32_MAX, INT32_MAX, INT32_MAX);
  GridTr_find_exts(collider->ps, collider->edge_count, &min, &max);
  if (lrc_min) {
    *lrc_min = GridTr_get_grid_cell_p(min, cell_size);
    lrc_min->x--;
    lrc_min->y--;
    lrc_min->z--;
  }
  if (lrc_max) {
    *lrc_max = GridTr_get_grid_cell_p(max, cell_size);
    lrc_max->x++;
    lrc_max->y++;
    lrc_max->z++;
  }
}
