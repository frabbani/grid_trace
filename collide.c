#include "collide.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

bool GridTr_sat_olap(const struct GridTr_sat_s *sat) {
  float a0 = sat->min_maxs[0].x, a1 = sat->min_maxs[0].y;
  float b0 = sat->min_maxs[1].x, b1 = sat->min_maxs[1].y;

  SORT2(a0, a1);
  SORT2(b0, b1);

  // overlap (touching counts)
  return !(a1 < (b0 - TOL) || b1 < (a0 - TOL));
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

void GridTr_collider_dtor(void *ptr) {
  GridTr_destroy_collider((struct GridTr_collider_s *)ptr);
}

void GridTr_create_collider(struct GridTr_collider_s *collider, uint id,
                            const struct vec3_s *ps, uint nps,
                            struct GridTr_plane_s plane) {
  if (!collider || !ps || nps < 3)
    return;

  collider->poly_id = id;
  collider->plane = plane;
  collider->edge_count = nps;
  collider->ps = GridTr_new(nps * sizeof(struct vec3_s));
  collider->es = GridTr_new(nps * sizeof(struct vec3_s));
  collider->edge_planes = GridTr_new(nps * sizeof(struct GridTr_plane_s));
  collider->edge_dists = GridTr_new(nps * sizeof(float));
  for (uint i = 0; i < nps; i++) {
    collider->ps[i] = ps[i];
    collider->es[i] = point_vec(collider->ps[i], collider->ps[(i + 1) % nps]);
    collider->edge_dists[i] = vec3_lensq(collider->es[i]);
    if (collider->edge_dists[i] > TOL_SQ) {
      collider->edge_dists[i] = sqrtf(collider->edge_dists[i]);
      collider->es[i] =
          vec3_mul(collider->es[i], 1.0f / collider->edge_dists[i]);
      collider->edge_planes[i] = GridTr_create_plane(
          vec3_cross(collider->plane.n, collider->es[i]), collider->ps[i]);
    } else {
      collider->edge_dists[i] = 0.0f;
      collider->es[i] = vec3_zero();
      collider->edge_planes[i].n = vec3_zero();
      collider->edge_planes[i].dist = 0.0f;
    }
  }
}

void GridTr_destroy_collider(struct GridTr_collider_s *collider) {
  if (!collider)
    return;
  GridTr_free(collider->ps);
  GridTr_free(collider->es);
  GridTr_free(collider->edge_planes);
  GridTr_free(collider->edge_dists);
  memset(collider, 0, sizeof(struct GridTr_collider_s));
}

bool GridTr_collider_touches_aabb(const struct GridTr_collider_s *collider,
                                  const struct GridTr_aabb_s *aabb) {
  struct vec3_s axes[3] = {
      {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};

  struct GridTr_sat_s sat;
#define TEST_SAT                                                               \
  do {                                                                         \
    GridTr_sat_setas(&sat, aabb->o, axes, aabb->halfsize, true);               \
    GridTr_sat_setps(&sat, collider->ps, collider->edge_count, false);         \
    if (!GridTr_sat_olap(&sat))                                                \
      return false;                                                            \
  } while (0)

  for (int i = 0; i < 3; i++) {
    sat.d = axes[i];
    TEST_SAT;
  }

  sat.d = collider->plane.n;
  TEST_SAT;

  // for (int j = 0; j < collider->edge_count; j++) {
  //   sat.d = collider->edge_planes[j].n;
  //   TEST_SAT;
  // }

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