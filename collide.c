#include "collide.h"
#include "vec.inl"

#include <math.h>
#include <stdio.h>
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

void GridTr_create_collider(struct GridTr_collider_s *collider, uint32 id,
                            const struct vec3_s *ps, uint32 nps,
                            struct GridTr_plane_s plane) {
  if (!collider || !ps || nps < 3)
    return;
  // printf("<%s>\n", __FUNCTION__);
  collider->poly_id = id;
  collider->plane = plane;
  collider->edge_count = nps;
  collider->ps = GridTr_new(nps * sizeof(struct vec3_s));
  collider->es = GridTr_new(nps * sizeof(struct vec3_s));
  collider->edge_planes = GridTr_new(nps * sizeof(struct GridTr_plane_s));
  collider->o = ps[0];
  for (uint i = 1; i < nps; i++) {
    collider->o = vec3_add(collider->o, ps[i]);
  }
  collider->o = vec3_mul(collider->o, 1.0f / (float)nps);
  // printf(" * plane detail: n=<%f, %f, %f> dist=%f\n", collider->plane.n.x,
  //        collider->plane.n.y, collider->plane.n.z, collider->plane.dist);

  collider->edge_lens = GridTr_new(nps * sizeof(float));
  for (uint i = 0; i < nps; i++) {
    collider->ps[i] = ps[i];
    collider->es[i] = point_vec(ps[i], ps[(i + 1) % nps]);
    collider->edge_lens[i] = vec3_lensq(collider->es[i]);
    // printf(" * edge: (%u -> %u)\n", i, (i + 1) % nps);
    if (collider->edge_lens[i] > TOL_SQ) {
      collider->edge_lens[i] = sqrtf(collider->edge_lens[i]);
      collider->es[i] =
          vec3_mul(collider->es[i], 1.0f / collider->edge_lens[i]);
      collider->edge_planes[i] = GridTr_create_plane(
          vec3_cross(collider->es[i], collider->plane.n), collider->ps[i]);

      // printf("    + length=%f d=<%f, %f, %f>\n", collider->edge_lens[i],
      //        collider->es[i].x, collider->es[i].y, collider->es[i].z);
      // bool inside = vec3_dot(point_vec(collider->o, collider->ps[i]),
      //                        collider->edge_planes[i].n) < 0;
      // printf("    + plane detail: n=<%f, %f, %f> dist=%f (%s)\n",
      //        collider->edge_planes[i].n.x, collider->edge_planes[i].n.y,
      //        collider->edge_planes[i].n.z, collider->edge_planes[i].dist,
      //        inside ? "inside" : "outside");
    } else {
      printf("<%s> - poly %u edge: %u: degenerate\n", __FUNCTION__,
             collider->poly_id, i);
      collider->edge_lens[i] = 0.0f;
      collider->es[i] = vec3_zero();
      collider->edge_planes[i].n = vec3_zero();
      collider->edge_planes[i].dist = 0.0f;
    }
  }
  // printf("---\n");
}

void GridTr_destroy_collider(struct GridTr_collider_s *collider) {
  if (!collider)
    return;
  GridTr_free(collider->ps);
  GridTr_free(collider->es);
  GridTr_free(collider->edge_planes);
  GridTr_free(collider->edge_lens);
  // memset(collider, 0, sizeof(struct GridTr_collider_s));
}

void GridTr_copy_collider(struct GridTr_collider_s *to,
                          const struct GridTr_collider_s *from) {
  if (to == NULL || from == NULL)
    return;
  // printf("<%s>\n", __FUNCTION__);
  to->poly_id = from->poly_id;
  to->plane = from->plane;
  to->o = from->o;
  to->radius = from->radius;
  to->edge_count = from->edge_count;
  to->ps = GridTr_new(from->edge_count * sizeof(struct vec3_s));
  to->es = GridTr_new(from->edge_count * sizeof(struct vec3_s));
  to->edge_lens = GridTr_new(from->edge_count * sizeof(float));
  to->edge_planes =
      GridTr_new(from->edge_count * sizeof(struct GridTr_plane_s));
  for (uint i = 0; i < from->edge_count; i++) {
    to->ps[i] = from->ps[i];
    to->es[i] = from->es[i];
    to->edge_lens[i] = from->edge_lens[i];
    to->edge_planes[i] = from->edge_planes[i];
    // printf(" * edge %u: plane detail: n=<%f, %f, %f> dist=%f\n", i,
    //        to->edge_planes[i].n.x, to->edge_planes[i].n.y,
    //        to->edge_planes[i].n.z, to->edge_planes[i].dist);
  }
  // printf("---\n");
}

bool GridTr_collider_touches_aabb(const struct GridTr_collider_s *collider,
                                  const struct GridTr_aabb_s *aabb) {
  struct vec3_s axes[3] = {
      {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};

  struct vec3_s aabb_ps[8];
  struct vec3_s size = vec3_mul(aabb->halfsize, 2.0f);
  // aabb_ps[0] = aabb->min;
  // aabb_ps[1] = vec3_add(aabb->min, vec3_set(size.x, 0, 0));
  // aabb_ps[2] = vec3_add(aabb->min, vec3_set(0, size.y, 0));
  // aabb_ps[3] = vec3_add(aabb->min, vec3_set(size.x, size.y, 0));
  // aabb_ps[4] = vec3_add(aabb->min, vec3_set(0, 0, size.z));
  // aabb_ps[5] = vec3_add(aabb->min, vec3_set(size.x, 0, size.z));
  // aabb_ps[6] = vec3_add(aabb->min, vec3_set(0, size.y, size.z));
  // aabb_ps[7] = vec3_add(aabb->min, vec3_set(size.x, size.y, size.z));
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

  for (int j = 0; j < collider->edge_count; j++) {
    sat.d = collider->edge_planes[j].n;
    TEST_SAT;
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < collider->edge_count; j++) {
      sat.d = vec3_cross(axes[i], collider->es[j]);
      if (vec3_lensq(sat.d) >= TOL_SQ) {
        sat.d = vec3_norm(sat.d);
        TEST_SAT;
      }
    }
  }
#undef TEST_SAT

  return true;
}

bool GridTr_load_colliders_from_obj(struct GridTr_collider_s **colliders,
                                    uint32 *num_colliders,
                                    const char *filename) {
  if (!colliders || !num_colliders || !filename) {
    printf("<%s> - missing parameter(s) (file '%s')\n", __FUNCTION__, filename);
    return false;
  }
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    printf("<%s> - Failed to open OBJ file '%s'\n", __FUNCTION__, filename);
    return false;
  }

  // printf("<%s> - Loading colliders from OBJ file '%s'\n", __FUNCTION__,
  //        filename);

  uint32 num_vs = 0;
  uint32 num_fs = 0;
  char line[256];
  while (fgets(line, sizeof(line), fp)) {
    // Parse the line and extract vertex/face information
    if (line[0] == 'v' && line[1] == ' ') {
      ++num_vs;
    } else if (line[0] == 'f' && line[1] == ' ') {
      ++num_fs;
    }
  }
  fseek(fp, 0, SEEK_SET);

  struct vec3_s *vs = GridTr_new(sizeof(struct vec3_s) * num_vs);
  *colliders = GridTr_new(sizeof(struct GridTr_collider_s) * num_fs);
  *num_colliders = num_fs;

  int i = 0;
  while (fgets(line, sizeof(line), fp)) {
    // Parse the line and extract vertex/face information
    if (line[0] == 'v' && line[1] == ' ') {
      struct vec3_s *v = &vs[i++];
      // Parse vertex information
      sscanf(line + 2, "%f %f %f", &v->x, &v->y, &v->z);
    }
  }
  fseek(fp, 0, SEEK_SET);

  i = 0;
  while (fgets(line, sizeof(line), fp)) {
    if (line[0] == 'f' && line[1] == ' ') {
      // Parse face information and create a collider
      struct GridTr_collider_s *collider = &(*colliders)[i++];
      struct vec3_s ps[8];
      uint num_ps = 0;

      char *ptr;
      char *tok = GridTr_strtok_r(line + 2, " ", &ptr);
      while (tok) {
        for (char *p = tok; *p != '\0'; p++) {
          if (*p == '/') {
            *p = '\0';
            break;
          }
        }
        int idx = atoi(tok) - 1;
        ps[num_ps++] = vs[idx];
        tok = GridTr_strtok_r(NULL, " \n", &ptr);
      }
      struct vec3_s u, v;
      u = point_vec(ps[0], ps[1]);
      v = point_vec(ps[0], ps[2]);
      struct GridTr_plane_s plane =
          GridTr_create_plane(vec3_cross(u, v), ps[0]);
      GridTr_create_collider(collider, i, ps, num_ps, plane);
      // printf(" * collider %d: %d edges | plane: <%.4f, %.4f, %.4f | %.4f>\n",
      // i,
      //        collider->edge_count, plane.n.x, plane.n.y, plane.n.z,
      //        plane.dist);
    }
  }
  GridTr_free(vs);

  // printf("<%s> - %u colliders created from OBJ file '%s'\n", __FUNCTION__,
  //        num_fs, filename);
  fclose(fp);
  return true;
}