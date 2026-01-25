#include "grid.h"
#include "vec.inl"

#include <math.h>
#include <stdio.h>
#include <string.h>

extern bool GridTr_debug_enabled();

#define TIE_EPS(t) (TOL * (1.0f + (t)))

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

void GridTr_get_exts_for_grid_cell(struct ivec3_s crl, float cell_size,
                                   struct vec3_s *min, struct vec3_s *max) {
  if (!min || !max) {
    return;
  }
  float x = (float)crl.x * cell_size;
  float y = (float)crl.y * cell_size;
  float z = (float)crl.z * cell_size;
  min->x = x;
  min->y = y;
  min->z = z;
  max->x = x + cell_size;
  max->y = y + cell_size;
  max->z = z + cell_size;
}

void GridTr_get_aabb_for_grid_cell(struct ivec3_s crl, float cell_size,
                                   struct GridTr_aabb_s *aabb) {
  if (!aabb) {
    return;
  }
  struct vec3_s min, max;
  float x = (float)crl.x * cell_size;
  float y = (float)crl.y * cell_size;
  float z = (float)crl.z * cell_size;
  min.x = x;
  min.y = y;
  min.z = z;
  max.x = x + cell_size;
  max.y = y + cell_size;
  max.z = z + cell_size;
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
  int total = 0;
  GridTr_get_collider_grid_cell_exts(collider, grid->cell_size, &crl_min,
                                     &crl_max, true);
  for (int z = crl_min.z; z <= crl_max.z; z++) {
    for (int y = crl_min.y; y <= crl_max.y; y++) {
      for (int x = crl_min.x; x <= crl_max.x; x++) {
        crl = ivec3_set(x, y, z);
        GridTr_get_aabb_for_grid_cell(crl, grid->cell_size, &aabb);
        if (!GridTr_collider_touches_aabb(collider, &aabb))
          continue;
        struct GridTr_grid_cell_s *cell = GridTr_grid_get_grid_cell(grid, crl);
        if (cell) {
          // printf(" * %s - adding collider %u to <%d, %d, %d>\n",
          // __FUNCTION__,
          //        collider->poly_id, cell->crl.x, cell->crl.y, cell->crl.z);
          GridTr_grid_cell_add_collider_idx(cell, idx);
          total++;
        } else {
          printf("<%s> - why was this cell not allocated???\n", __FUNCTION__);
        }
      }
    }
  }
  // printf("<%s> - collider %u touched %d cells\n", __FUNCTION__,
  //        collider->poly_id, total);
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

// GridTr_step_ray_through_grid_cell:
// * assumes rayseg->o is inside the cell defined by crl
// * clips the ray end to the cell boundaries
static bool GridTr_step_ray_through_grid_cell(const struct GridTr_grid_s *grid,
                                              struct GridTr_rayseg_s *rayseg,
                                              struct ivec3_s *crl) {

  const float dir_eps = 1e-12f;

  const float dx = rayseg->d.x;
  const float dy = rayseg->d.y;
  const float dz = rayseg->d.z;

  if (fabsf(dx) <= dir_eps && fabsf(dy) <= dir_eps && fabsf(dz) <= dir_eps) {
    // no ray!!!
    return false;
  }

  struct vec3_s cmin, cmax;
  GridTr_get_exts_for_grid_cell(*crl, grid->cell_size, &cmin, &cmax);

  // Distance along ray (t) to the next boundary on each axis
  float tx = INFINITY, ty = INFINITY, tz = INFINITY;

  if (dx > dir_eps)
    tx = (cmax.x - rayseg->o.x) / dx;
  else if (dx < -dir_eps)
    tx = (cmin.x - rayseg->o.x) / dx; // dx negative => positive tx

  if (dy > dir_eps)
    ty = (cmax.y - rayseg->o.y) / dy;
  else if (dy < -dir_eps)
    ty = (cmin.y - rayseg->o.y) / dy;

  if (dz > dir_eps)
    tz = (cmax.z - rayseg->o.z) / dz;
  else if (dz < -dir_eps)
    tz = (cmin.z - rayseg->o.z) / dz;

  float t = tx;
  if (ty < t)
    t = ty;
  if (tz < t)
    t = tz;
  if (t < 0.0f)
    t = 0.0f;

  if (t >= rayseg->len) {
    return false;
  }

  rayseg->e = vec3_add(rayseg->o, vec3_mul(rayseg->d, t));
  rayseg->len = t;

  // Step cell indices. If ties, we crossed an edge/corner -> step multiple
  // axes.
  float eps = TIE_EPS(t);
  if (fabsf(tx - t) <= eps) {
    crl->x += (dx < 0.0f) ? -1 : 1;
  }
  if (fabsf(ty - t) <= eps) {
    crl->y += (dy < 0.0f) ? -1 : 1;
  }
  if (fabsf(tz - t) <= eps) {
    crl->z += (dz < 0.0f) ? -1 : 1;
  }
  return true;
}

// returns true if cb returns true (early exit)
bool GridTr_trace_ray_through_grid(const struct GridTr_grid_s *grid,
                                   const struct GridTr_rayseg_s *rayseg,
                                   GridTr_trace_cb cb, void *user_data) {
  if (!cb || !grid || !rayseg) {
    printf("<%s> - invalid argument(s)\n", __FUNCTION__);
    return false;
  }
  float eps = grid->cell_size * 1e-5f;

  if (rayseg->len <= eps) {
    return false;
  }
  const struct GridTr_collider_s *colliders = grid->colliders->data;

  struct vec3_s o = rayseg->o;
  float remaining = rayseg->len;
  struct ivec3_s crl = GridTr_get_grid_cell_for_p(rayseg->o, grid->cell_size);

  bool debug = false; // GridTr_debug_enabled();
  if (debug) {
    printf("<%s> - begin tracing ray\n", __FUNCTION__);
  }
  int steps = 0;
  while (remaining >= eps) {
    ++steps;
    struct GridTr_rayseg_s r = {0};
    r.o = o;
    r.d = rayseg->d;
    r.e = rayseg->e;
    r.len = remaining;
    struct ivec3_s crl_next = crl;
    if (debug) {
      printf(" --- [%d, %d, %d] (%f remaining)\n", crl.x, crl.y, crl.z,
             remaining);
    }
    bool hit_boundary = GridTr_step_ray_through_grid_cell(grid, &r, &crl_next);
    if (debug) {
      printf(" +++ stepped to [%d, %d, %d] (%f len)\n", crl_next.x, crl_next.y,
             crl_next.z, r.len);
    }
    const struct GridTr_grid_cell_s *cell =
        GridTr_grid_get_grid_cell_ro(grid, crl);
    if (cb(cell, crl, &r, colliders, user_data)) {
      return true;
    }
    if (!hit_boundary) {
      return false;
    }
    // float nudge_eps = TIE_EPS(r.len);
    o = r.e; // vec3_add(r.e, vec3_mul(r.d, nudge_eps));
    remaining -= r.len;
    crl = crl_next;
  }
  if (debug) {
    printf("<%s> - finished tracing ray (%d steps)\n", __FUNCTION__, steps);
  }

  return false;
}
