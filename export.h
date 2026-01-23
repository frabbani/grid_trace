#pragma once

#include "grid.h"

struct GridTr_shape_s {
  struct vec3_s *ps;
  struct ivec3_s *faces; // assumes triangle faces for now
  uint32 num_ps, num_faces;
};

void GridTr_load_shape_from_obj(struct GridTr_shape_s *shape,
                                const char *filename);

char *GridTr_export_shape_to_obj_str(const struct GridTr_shape_s *shape,
                                     struct vec3_s o, float scale,
                                     uint32 starting_vertex);

void GridTr_free_shape(struct GridTr_shape_s *shape);

void GridTr_export_grid_boxes_to_obj(const struct GridTr_grid_s *grid,
                                     const char *filename);

bool GridTr_load_colliders_from_obj(struct GridTr_collider_s **colliders,
                                    uint32 *num_colliders,
                                    const char *filename);
