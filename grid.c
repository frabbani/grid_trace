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

static void GridTr_grid_cell_add_collider_idx(struct GridTr_grid_cell_s *cell,
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
  GridTr_free(cell); // this is to free the cell itself
}

struct ivec3_s GridTr_get_grid_cell_for_p(struct vec3_s p, float cell_size) {
  struct ivec3_s crl;
  float s = 1.0f / cell_size;
  crl.x = (int32)floorf(p.x * s);
  crl.y = (int32)floorf(p.y * s);
  crl.z = (int32)floorf(p.z * s);
  return crl;
}

void GridTr_get_aabb_for_grid_cell(struct ivec3_s crl, float cell_size,
                                   struct GridTr_aabb_s *aabb) {
  if (!aabb) {
    return;
  }
  struct vec3_s min, max;
  min.x = crl.x * cell_size;
  min.y = crl.y * cell_size;
  min.z = crl.z * cell_size;
  max.x = (crl.x + 1) * cell_size;
  max.y = (crl.y + 1) * cell_size;
  max.z = (crl.z + 1) * cell_size;
  GridTr_aabb_init(aabb, min, max);
}

void GridTr_get_collider_grid_cell_exts(
    const struct GridTr_collider_s *collider, float cell_size,
    struct ivec3_s *crl_min, struct ivec3_s *crl_max, bool bloat) {
  struct vec3_s min, max;
  GridTr_find_exts(collider->ps, collider->edge_count, &min, &max);
  if (crl_min) {
    *crl_min = GridTr_get_grid_cell_for_p(min, cell_size);
    if (bloat) {
      crl_min->x--;
      crl_min->y--;
      crl_min->z--;
    }
  }
  if (crl_max) {
    *crl_max = GridTr_get_grid_cell_for_p(max, cell_size);
    if (bloat) {
      crl_max->x++;
      crl_max->y++;
      crl_max->z++;
    }
  }
}

void GridTr_add_collider_to_grid(struct GridTr_grid_s *grid,
                                 const struct GridTr_collider_s *collider) {
  if (!grid || !collider) {
    printf("<%s> - invalid grid or collider\n", __FUNCTION__);
    return;
  }
  struct GridTr_collider_s blank = {0};
  GridTr_array_add(grid->colliders, &blank);
  uint32 idx = grid->colliders->num_elems - 1;
  GridTr_copy_collider(GridTr_array_get(grid->colliders, idx), collider);

  struct ivec3_s crl_min, crl_max, crl;
  struct GridTr_aabb_s aabb;
  GridTr_get_collider_grid_cell_exts(collider, grid->cell_size, &crl_min,
                                     &crl_max, true);
  for (int z = crl_min.z; z <= crl_max.z; z++) {
    for (int y = crl_min.y; y <= crl_max.y; y++) {
      for (int x = crl_min.x; x <= crl_max.x; x++) {
        crl = ivec3_set(x, y, z);
        GridTr_get_aabb_for_grid_cell(crl, grid->cell_size, &aabb);
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

void GridTr_destroy_grid(struct GridTr_grid_s *grid) {
  if (!grid) {
    return;
  }
  GridTr_destroy_hash_table(&grid->cell_table);
  GridTr_destroy_array_dtor(&grid->colliders, GridTr_collider_dtor);
  grid->cell_size = 0.0f;
}

struct GridTr_grid_cell_s *GridTr_grid_get_grid_cell(struct GridTr_grid_s *grid,
                                                     struct ivec3_s crl) {
  if (!grid) {
    return NULL;
  }
  uint64 hash = ivec3_fnv1a(crl);
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
      (*cell)->crl = crl;
      (*cell)->hash = hash;
      (*cell)->num_colliders = 0;
      (*cell)->colliders = GridTr_new(16 * sizeof(uint32));
      (*cell)->_max_colliders_ = 16;
      GridTr_get_aabb_for_grid_cell(crl, grid->cell_size, &(*cell)->aabb);
      return *cell;
    }
  }

  return NULL;
}

const struct GridTr_grid_cell_s *
GridTr_grid_get_grid_cell_ro(const struct GridTr_grid_s *grid,
                             struct ivec3_s crl) {
  if (!grid) {
    return NULL;
  }
  uint64 hash = ivec3_fnv1a(crl);
  const struct GridTr_grid_cell_s *cell =
      (const struct GridTr_grid_cell_s *)GridTr_hash_table_maybe_get_ro(
          grid->cell_table, hash);
  return cell;
}

const void **GridTr_grid_get_all_grid_cells(const struct GridTr_grid_s *grid,
                                            uint32 *num_cells) {
  if (!grid || !num_cells) {
    return NULL;
  }
  return GridTr_hash_table_get_all_ro(grid->cell_table, num_cells);
}