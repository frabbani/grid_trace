#pragma once

#include "collide.h"
#include "hash.h"

struct GridTr_grid_cell_s {
  struct ivec3_s lrc; // z := layer, y := row, x := column
  uint64 hash;
  uint32 num_colliders;
  uint32 *colliders;
  uint32 _max_colliders_;
  struct GridTr_aabb_s aabb;
};

struct GridTr_grid_s {
  struct GridTr_hash_table_s *cell_table;
  struct GridTr_array_s *colliders;
  uint32 cell_size;
  struct GridTr_aabb_s aabb;
};

void GridTr_grid_cell_add_collider_idx(struct GridTr_grid_cell_s *cell,
                                       uint32 collider_idx);
void GridTr_grid_cell_dtor(void *ptr);
struct ivec3_s GridTr_get_grid_cell_for_p(struct vec3_s p, float cell_size);
void GridTr_get_aabb_for_grid_cell(struct ivec3_s lrc, float cell_size,
                                   struct GridTr_aabb_s *aabb);
void GridTr_get_collider_grid_cell_exts(
    const struct GridTr_collider_s *collider, float cell_size,
    struct ivec3_s *lrc_min, struct ivec3_s *lrc_max, bool bloat);
void GridTr_create_grid(struct GridTr_grid_s *grid, float cell_size);
struct GridTr_grid_cell_s *GridTr_grid_get_grid_cell(struct GridTr_grid_s *grid,
                                                     struct ivec3_s lrc);

const struct GridTr_grid_cell_s *
GridTr_grid_get_grid_cell_ro(const struct GridTr_grid_s *grid,
                             struct ivec3_s lrc);
void GridTr_add_collider_to_grid(struct GridTr_grid_s *grid,
                                 const struct GridTr_collider_s *collider);

struct GridTr_trace_s {
  struct GridTr_rayseg_s rayseg;
  struct vec3_s p;
  float t;
  struct ivec3_s cell;
  struct ivec3_s starting_cell;
  uint32 collider;
};

void GridTr_grid_trace(const struct GridTr_grid_s *grid,
                       struct GridTr_trace_s *trace);