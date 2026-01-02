#pragma once
#include "geom.h"

#include <math.h>

struct GridTr_sat_s {
  struct vec3_s d;
  struct vec2_s min_maxs[2];
};

bool GridTr_sat_olap(const struct GridTr_sat_s *sat) {
  float a0 = sat->min_maxs[0].x, a1 = sat->min_maxs[0].y;
  float b0 = sat->min_maxs[1].x, b1 = sat->min_maxs[1].y;

  SORT2(a0, a1);
  SORT2(b0, b1);

  // overlap (touching counts)
  return !(a1 < b0 || b1 < a0);
}

void GridTr_sat_setps(struct GridTr_sat_s *sat, const struct vec3_s *ps,
                      uint num_ps, bool first) {
  if (!sat || !ps || num_ps == 0)
    return;

  struct vec2_s *p = first ? &sat->min_maxs[0] : &sat->min_maxs[1];
  p->x = p->y = vec3_dot(ps[0], sat->d);
  for (uint i = 1; i < num_ps; i++) {
    float proj = vec3_dot(ps[i], sat->d);
    p->x = MIN(p->x, proj);
    p->y = MAX(p->y, proj);
  }
}

void GridTr_sat_setr(struct GridTr_sat_s *sat, struct vec3_s c, float radius,
                     bool first) {
  if (!sat)
    return;

  struct vec2_s *p = first ? &sat->min_maxs[0] : &sat->min_maxs[1];
  float m = vec3_dot(c, sat->d);
  p->x = m - radius;
  p->y = m + radius;
}

void GridTr_sat_setas(struct GridTr_sat_s *sat, struct vec3_s o,
                      const struct vec3_s *axes, struct vec3_s half_size,
                      bool first) {
  float m = vec3_dot(o, sat->d);
  float a = fabsf(vec3_dot(axes[0], sat->d) * half_size.x);
  float b = fabsf(vec3_dot(axes[1], sat->d) * half_size.y);
  float c = fabsf(vec3_dot(axes[2], sat->d) * half_size.z);
  // Update the SAT's min/max values
  struct vec2_s *p = first ? &sat->min_maxs[0] : &sat->min_maxs[1];
  float r = a + b + c;
  p->x = m - r;
  p->y = m + r;
}

void GridTr_set_sat(struct GridTr_sat_s *sat, struct vec3_s d) {
  sat->d = vec3_norm(d);
  sat->min_maxs[0] = sat->min_maxs[0] = vec2_set(0.0f, 0.0f);
}

struct GridTr_collider_s {
  uint poly_id;
  struct GridTr_plane_s plane;
  struct vec3_s o;
  float radius;
  uint edge_count;
  struct GridTr_plane_s *edge_planes;
  float *edge_dists;
  struct vec3_s *ps;
  struct vec3_s *es;
};

bool GridTr_collider_touches_aabb(const struct GridTr_collider_s *collider,
                                  const struct GridTr_aabb_s *aabb) {
  struct vec3_s axes[3] = {
      {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};

  struct GridTr_sat_s sat;
  // clang-format off
#define TEST_SAT \
do { \
  GridTr_sat_setas(&sat, aabb->o, axes, aabb->halfsize, true); \
  GridTr_sat_setps(&sat, collider->ps, collider->edge_count, false); \
  if (!GridTr_sat_olap(&sat)) return false; \
} while (0)
  // clang-format on

  for (int i = 0; i < 3; i++) {
    sat.d = axes[i];
    TEST_SAT;
  }

  sat.d = collider->plane.n;
  TEST_SAT;

  for (int j = 0; j < collider->edge_count; j++) {
    sat.d = collider->edge_planes[j].n;
    TEST_SAT;
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < collider->edge_count; j++) {
      sat.d = vec3_cross(axes[i], collider->es[j]);
      if (vec3_lensq(sat.d) >= TOL_SQ) {
        TEST_SAT;
      }
    }
  }
#undef TEST_SAT

  return true;
}