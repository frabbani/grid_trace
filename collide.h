#pragma once

#include "geom.h"

struct GridTr_sat_s {
  struct vec3_s d;
  struct vec2_s min_maxs[2];
};

bool GridTr_sat_olap(const struct GridTr_sat_s *sat);

void GridTr_sat_setps(struct GridTr_sat_s *sat, const struct vec3_s *ps,
                      uint num_ps, bool first);

void GridTr_sat_setr(struct GridTr_sat_s *sat, struct vec3_s c, float radius,
                     bool first);

void GridTr_sat_setas(struct GridTr_sat_s *sat, struct vec3_s o,
                      const struct vec3_s *axes, struct vec3_s half_size,
                      bool first);

struct GridTr_collider_s {
  uint poly_id;
  struct GridTr_plane_s plane;
  struct vec3_s o;
  float radius;
  uint edge_count;
  struct GridTr_plane_s *edge_planes;
  float *edge_lens;
  struct vec3_s *ps;
  struct vec3_s *es;
};

// assumes points are in counter-clockwise order and form a convex polygon
void GridTr_create_collider(struct GridTr_collider_s *collider, uint id,
                            const struct vec3_s *ps, uint nps,
                            struct GridTr_plane_s plane);

void GridTr_destroy_collider(struct GridTr_collider_s *collider);

bool GridTr_collider_touches_aabb(const struct GridTr_collider_s *collider,
                                  const struct GridTr_aabb_s *aabb);

void GridTr_copy_collider(struct GridTr_collider_s *to,
                          const struct GridTr_collider_s *from);

void GridTr_collider_dtor(void *ptr);