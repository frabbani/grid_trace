#include "grid.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/*
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
*/

void GridTr_grid_cell_add_collider_idx(struct GridTr_grid_cell_s *cell,
                                       uint32 collider_idx) {
  if (!cell)
    return;
  MAYBE_RESIZE_FIX(cell->colliders, cell->num_colliders, cell->_max_colliders_,
                   sizeof(uint32), 16);
  cell->colliders[cell->num_colliders++] = collider_idx;
}

void GridTr_grid_cell_dtor(void *ptr) {
  struct GridTr_grid_cell_s *cell = (struct GridTr_grid_cell_s *)ptr;
  if (!cell)
    return;
  GridTr_free(cell->colliders);
  GridTr_free(cell);
}

struct ivec3_s GridTr_get_grid_cell_for_p(struct vec3_s p, float cell_size) {
  struct ivec3_s lrc;
  float s = 1.0f / cell_size;
  lrc.x = (int32)floorf(p.x * s);
  lrc.y = (int32)floorf(p.y * s);
  lrc.z = (int32)floorf(p.z * s);
  return lrc;
}

void GridTr_get_aabb_for_grid_cell(struct ivec3_s lrc, float cell_size,
                                   struct GridTr_aabb_s *aabb) {
  if (!aabb) {
    return;
  }
  struct vec3_s min, max;
  min.x = lrc.x * cell_size;
  min.y = lrc.y * cell_size;
  min.z = lrc.z * cell_size;
  max.x = (lrc.x + 1) * cell_size;
  max.y = (lrc.y + 1) * cell_size;
  max.z = (lrc.z + 1) * cell_size;
  GridTr_aabb_init(aabb, min, max);
}

void GridTr_get_collider_grid_cell_exts(
    const struct GridTr_collider_s *collider, float cell_size,
    struct ivec3_s *lrc_min, struct ivec3_s *lrc_max, bool bloat) {
  struct vec3_s min, max;
  GridTr_find_exts(collider->ps, collider->edge_count, &min, &max);
  if (lrc_min) {
    *lrc_min = GridTr_get_grid_cell_for_p(min, cell_size);
    if (bloat) {
      lrc_min->x--;
      lrc_min->y--;
      lrc_min->z--;
    }
  }
  if (lrc_max) {
    *lrc_max = GridTr_get_grid_cell_for_p(max, cell_size);
    if (bloat) {
      lrc_max->x++;
      lrc_max->y++;
      lrc_max->z++;
    }
  }
}

void GridTr_add_collider_to_grid(struct GridTr_grid_s *grid,
                                 const struct GridTr_collider_s *collider) {
  if (!grid || !collider) {
    printf("<%s> - invalid grid or collider\n", __FUNCTION__);
    return;
  }
  GridTr_array_add(grid->colliders, collider);
  uint32 idx = grid->colliders->num_elems - 1;
  struct ivec3_s lrc_min, lrc_max, lrc;
  struct GridTr_aabb_s aabb;
  GridTr_get_collider_grid_cell_exts(collider, grid->cell_size, &lrc_min,
                                     &lrc_max, true);
  for (int z = lrc_min.z; z <= lrc_max.z; z++) {
    for (int y = lrc_min.y; y <= lrc_max.y; y++) {
      for (int x = lrc_min.x; x <= lrc_max.x; x++) {
        lrc = ivec3_set(x, y, z);
        GridTr_get_aabb_for_grid_cell(lrc, grid->cell_size, &aabb);
        if (!GridTr_collider_touches_aabb(collider, &aabb))
          continue;
        struct GridTr_grid_cell_s *cell =
            GridTr_grid_get_grid_cell(grid, (struct ivec3_s){x, y, z});
        if (cell) {
          GridTr_grid_cell_add_collider_idx(cell, idx);
        } else {
          printf("<%s> - why was this cell not allocated???\n", __FUNCTION__);
        }
      }
    }
  }
}

void GridTr_create_grid(struct GridTr_grid_s *grid, float cell_size) {
  if (!grid) {
    return;
  }
  grid->cell_size = cell_size;
  grid->cell_table = GridTr_create_hash_table(256, GridTr_grid_cell_dtor);
  grid->colliders =
      GridTr_create_array(sizeof(struct GridTr_collider_s), 4096, 4096);
  grid->colliders->oftype = GridTr_oftype(struct GridTr_collider_s);
  GridTr_aabb_init(&grid->aabb, vec3_zero(), vec3_zero());
}

struct GridTr_grid_cell_s *GridTr_grid_get_grid_cell(struct GridTr_grid_s *grid,
                                                     struct ivec3_s lrc) {
  if (!grid) {
    return NULL;
  }
  uint64 hash = ivec3_fnv1a(lrc);
  struct GridTr_grid_cell_s **cell =
      (struct GridTr_grid_cell_s **)GridTr_hash_table_maybe_get(
          grid->cell_table, hash);
  if (cell) {
    return *cell;
  }
  cell = (struct GridTr_grid_cell_s **)GridTr_hash_table_add_or_get(
      grid->cell_table, hash);
  if (cell) {
    *cell = GridTr_new(sizeof(struct GridTr_grid_cell_s));
    if (*cell) {
      (*cell)->lrc = lrc;
      (*cell)->hash = hash;
      (*cell)->num_colliders = 0;
      (*cell)->colliders = GridTr_new(16 * sizeof(uint32));
      (*cell)->_max_colliders_ = 16;
      GridTr_get_aabb_for_grid_cell(lrc, grid->cell_size, &(*cell)->aabb);
      return *cell;
    }
  }

  return NULL;
}

const struct GridTr_grid_cell_s *
GridTr_grid_get_grid_cell_ro(const struct GridTr_grid_s *grid,
                             struct ivec3_s lrc) {
  if (!grid) {
    return NULL;
  }
  uint64 hash = ivec3_fnv1a(lrc);
  const struct GridTr_grid_cell_s *cell =
      (const struct GridTr_grid_cell_s *)GridTr_hash_table_maybe_get_ro(
          grid->cell_table, hash);
  return cell;
}